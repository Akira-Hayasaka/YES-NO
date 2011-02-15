#include "testApp.h"
#include "ofxSimpleGuiToo.h"

/*
 ** フレームバッファオブジェクト
 */
static GLuint fbo;                            // フレームバッファオブジェクト名
static const GLenum bufs[] = {                // バッファ名
	GL_COLOR_ATTACHMENT0_EXT,                   //   拡散反射光
	GL_COLOR_ATTACHMENT1_EXT,                   //   鏡面反射光
	GL_COLOR_ATTACHMENT2_EXT,                   //   位置
	GL_COLOR_ATTACHMENT3_EXT,                   //   法線
};
#define BUFNUM (sizeof bufs / sizeof bufs[0]) // アタッチメントの数
#define BUFWIDTH 1024                         // フレームバッファオブジェクトの幅
#define BUFHEIGHT 1024                        // フレームバッファオブジェクトの高さ
#define TEXNUM (BUFNUM + 3)                   // テクスチャの数 (buf の要素 + 深度 + 環境 + 背景)
static GLuint tex[TEXNUM];                    // テクスチャ名 
static GLint unit;                            // テクスチャユニットの uniform 変数

/*
 ** 深度のサンプリングに使う点群
 */
#define MAXSAMPLES 256                        // サンプル点の最大数
#define SAMPLERADIUS 0.1f                     // サンプル点の散布半径
static GLfloat pointbuf[MAXSAMPLES][4];       // サンプル点の位置
static GLint point;                           // サンプル点の uniform 変数

/*
 ** シェーダオブジェクト
 */
static GLuint pass1, pass2;

/*
 **トラックボール処理
 */
static Trackball *tb;
static int btn;

/*
 ** OBJ ファイル
 */
static Obj *obj;
static char objfile[] = "/Users/makira/Project/20110321.VancouverMusium/oF0062/apps/SandBox/ssao1/src/bunny.obj";

/*
 ** 材質
 */
static const GLfloat kd[] = { 0.5f, 0.5f, 0.1f, 0.6f };     // 拡散反射係数 (alpha > 0 にする)
static const GLfloat ks[] = { 0.2f, 0.2f, 0.2f, 0.0f };     // 鏡面反射係数 (alpha = 0 にする)
static const GLfloat kshi = 40.0f;                          // 輝き係数
static const GLfloat kr[] = { 0.2f, 0.2f, 0.2f, 0.0f };     // 映り込みの反射率 (alpha = 0 にする)
static const GLfloat keta = 0.67f;                          // 屈折率の比
static const GLfloat kbgd = 0.1f;                           // 背景との仮想的な距離
static GLint reflection;                                    // 映り込みの反射率を渡す uniform 変数
static GLint refraction;                                    // 屈折率の比を渡す uniform 変数
static GLint bgdistance;                                    // 背景との距離を渡す uniform 変数

/*
 ** 光源
 */
static const GLfloat lpos[] = { 0.0f, 0.0f, 10.0f, 1.0f };  // 光源位置 (w は無視している)
static const GLfloat lcol[] = { 0.5f, 0.5f, 0.5f, 1.0f };   // 光源強度 (alpha = 1 にする)
static const GLfloat lamb[] = { 0.8f, 0.8f, 0.8f, 0.0f };   // 環境光強度 (alpha = 0 にする)

/*
 ** 視点
 */
static GLdouble fovy = 60.0, zNear = 0.5, zFar = 3.0;
//1024,768
static GLdouble ex = 0.0, ey = 0.0, ez = -1.5;
static const GLdouble shiftstep = 0.1;  

/*
 ** シェーダプログラムの作成
 */
static GLuint loadShader(const char *vert, const char *frag)
{
	// シェーダオブジェクトの作成
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	// シェーダのソースプログラムの読み込み
	if (readShaderSource(vertShader, vert)) exit(1);
	if (readShaderSource(fragShader, frag)) exit(1);
	
	// シェーダプログラムのコンパイル／リンク結果を得る変数
	GLint compiled, linked;
	
	// バーテックスシェーダのソースプログラムのコンパイル
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
	printShaderInfoLog(vertShader);
	if (compiled == GL_FALSE) {
		std::cerr << "Compile error in vertex shader." << std::endl;
		exit(1);
	}
	
	// フラグメントシェーダのソースプログラムのコンパイル
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
	printShaderInfoLog(fragShader);
	if (compiled == GL_FALSE) {
		std::cerr << "Compile error in fragment shader." << std::endl;
		exit(1);
	}
	
	// プログラムオブジェクトの作成
	GLuint gl2Program = glCreateProgram();
	
	// シェーダオブジェクトのシェーダプログラムへの登録
	glAttachShader(gl2Program, vertShader);
	glAttachShader(gl2Program, fragShader);
	
	// シェーダオブジェクトの削除
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	
	// シェーダプログラムのリンク
	glLinkProgram(gl2Program);
	glGetProgramiv(gl2Program, GL_LINK_STATUS, &linked);
	printProgramInfoLog(gl2Program);
	if (linked == GL_FALSE) {
		std::cerr << "Link error." << std::endl;
		exit(1);
	}
	
	return gl2Program;
}

/*
 ** テクスチャファイルの読み込み
 */
static GLuint loadImage(const char *name, int width, int height)
{
	std::ifstream file(name, std::ios::binary);
	
	if (file.fail()) {
		std::cerr << "Can't open file: " << name << std::endl;
		exit(1);
	}
	else {
		
		// ファイルの末尾に移動
		file.seekg(0L, std::ios::end);
		
		// 現在のファイルの読み込み位置 (＝サイズ) を取得
		GLsizei length = file.tellg();
		
		// ファイルサイズ分のメモリを確保
		GLubyte *image = new GLubyte[length];
		
		// ファイルの先頭に移動
		file.seekg(0L, std::ios::beg);
		
		// ファイルの読み込み
		file.read((char *)image, length);
		
		// テクスチャオブジェクトの作成
		GLuint texname;
		glGenTextures(1, &texname);
		
		// テクスチャの結合
		glBindTexture(GL_TEXTURE_2D, texname);
		
		// テクスチャの割り当て
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		
		// テクスチャパラメータ
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		// 読み込みに使ったメモリの開放
		delete[] image;
		
		return texname;
	}
}

/*
 ** フレームバッファオブジェクトのアタッチメントの作成
 */
static GLuint attachTexture(GLint format, GLsizei width, GLsizei height, GLenum attachment)
{
	// テクスチャオブジェクトの作成
	GLuint texname;
	glGenTextures(1, &texname);
	
	// テクスチャの結合
	glBindTexture(GL_TEXTURE_2D, texname);
	
	// テクスチャの割り当て
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
				 (format == GL_DEPTH_COMPONENT) ? GL_DEPTH_COMPONENT : GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
	// テクスチャパラメータ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// フレームバッファオブジェクトに結合
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_2D, texname, 0);
	
	return texname;
}



/*
 ** ウィンドウの幅と高さ
 */
static int ww, wh;

//--------------------------------------------------------------
void testApp::setup(){
	
	ofDisableArbTex();
	
	ww = ofGetWidth();
	wh = ofGetHeight();

	// OBJ ファイルの読み込み
	obj = new Obj(objfile);
	
	// トラックボール処理用オブジェクトの生成
	tb = new Trackball;
	
	// シェーダプログラムの作成
	pass1 = loadShader("/Users/makira/Project/20110321.VancouverMusium/oF0062/apps/SandBox/ssao1/src/pass1.vert", "/Users/makira/Project/20110321.VancouverMusium/oF0062/apps/SandBox/ssao1/src/pass1.frag");
	pass2 = loadShader("/Users/makira/Project/20110321.VancouverMusium/oF0062/apps/SandBox/ssao1/src/pass2.vert", "/Users/makira/Project/20110321.VancouverMusium/oF0062/apps/SandBox/ssao1/src/pass2.frag");
	
	// pass2 の uniform 変数の識別子の取得
	unit = glGetUniformLocation(pass2, "unit");
	point = glGetUniformLocation(pass2, "point");
	reflection = glGetUniformLocation(pass2, "reflection");
	refraction = glGetUniformLocation(pass2, "refraction");
	bgdistance = glGetUniformLocation(pass2, "bgdistance");
	
	/*
	 ** フレームバッファオブジェクトを作成してレンダリングターゲットを用意する
	 */
	
	// フレームバッファオブジェクトの作成
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	
	// 拡散反射光 D + 鏡面反射光 S
	tex[0] = attachTexture(GL_RGBA, BUFWIDTH, BUFHEIGHT, bufs[0]);
	
	// 環境光の反射光 A
	tex[1] = attachTexture(GL_RGBA, BUFWIDTH, BUFHEIGHT, bufs[1]);
	
	// 位置 P
	tex[2] = attachTexture(GL_RGBA32F_ARB, BUFWIDTH, BUFHEIGHT, bufs[2]);
	
	// 法線 N
	tex[3] = attachTexture(GL_RGBA32F_ARB, BUFWIDTH, BUFHEIGHT, bufs[3]);
	
	// 深度
	tex[4] = attachTexture(GL_DEPTH_COMPONENT, BUFWIDTH, BUFHEIGHT, GL_DEPTH_ATTACHMENT_EXT);
	
	// フレームバッファオブジェクトの結合を解除
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	// レンダリングターゲットをクリアする値
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	//  // 環境 (スフィアマップ)
	//  tex[5] = loadImage("room.raw", 256, 256);
	//
	//  // 背景 (屈折マッピング)
	//  tex[6] = loadImage("campus.raw", 1024, 1024);
	
	// デフォルトのテクスチャに戻す
	glBindTexture(GL_TEXTURE_2D, 0);
	
	/*
	 ** 環境遮蔽係数を算出するのに使う球状の点群
	 */
	
	// サンプル点の生成
	for (int i = 0; i < MAXSAMPLES; ++i) {
		float r = SAMPLERADIUS * (float)rand() / (float)RAND_MAX;
		float t = 6.2831853f * (float)rand() / ((float)RAND_MAX + 1.0f);
		float cp = 2.0f * (float)rand() / (float)RAND_MAX - 1.0f;
		float sp = sqrt(1.0f - cp * cp);
		float ct = cos(t), st = sin(t);
		
		pointbuf[i][0] = r * sp * ct;
		pointbuf[i][1] = r * sp * st;
		pointbuf[i][2] = r * cp;
		pointbuf[i][3] = 0.0f;
	}	
	
	
	gui.addTitle("Setting");
	gui.loadFromXML();
	gui.show();		
}

/*
 ** 地面
 */
static void ground(void)
{
	static const GLfloat vertex[][3] = {
		{ -1.0f, -0.6f, -1.0f },
		{ -1.0f, -0.6f,  1.0f },
		{  1.0f, -0.6f,  1.0f },
		{  1.0f, -0.6f, -1.0f },
	};
	static const GLfloat normal[][3] = {
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
	};
	static const GLfloat color[][4] = {
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
	};
	
	// 材質の設定
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color[0]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color[1]);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 1.0f);
	
	// 地面のポリゴンの描画
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertex);
	glNormalPointer(GL_FLOAT, 0, normal);
	glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof vertex / sizeof vertex[0]);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

/*
 ** シーンの描画
 */
static void scene(void)
{
	// モデルビュー変換行列の保存
	glPushMatrix();
	
	// 地面を描く
	ground();
	
	// トラックボール式の回転を与える
	glMultMatrixd(tb->rotation());
	
	glRotated(45.0, 0.0, 1.0, 0.0);
	
	// 材質を設定する
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ks);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, kshi);
	
#if 0
	glBegin(GL_POINTS);
	for (int i = 0; i < MAXSAPLES; ++i) glVertex3fv(pointbuf[i]);
	glEnd();
#else
	// オブジェクトを描く
	obj->draw();
#endif
	
	// モデルビュー変換行列の復帰
	glPopMatrix();
}



//--------------------------------------------------------------
void testApp::update(){
	
}

//--------------------------------------------------------------
void testApp::draw(){
	
	/*
	 ** pass1: レンダリングターゲットに D, S, P, N および深度を格納する
	 */
	
	// テクスチャ全体をビューポートにする
	glViewport(0, 0, BUFWIDTH, BUFHEIGHT);
	
	// デプステストを有効にする
	glEnable(GL_DEPTH_TEST);
	
	// フレームバッファオブジェクトへのレンダリング開始
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glDrawBuffers(BUFNUM, bufs);
	
	// カラーバッファとデプスバッファをクリア
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// pass1 シェーダーを有効にする
	glUseProgram(pass1);
	
	// モデルビュー変換行列の設定
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(ex, ey, ez);
	
	// 光源
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lcol);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lcol);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lamb);
	
	// シーンの描画
	scene();
	
	/*
	 ** pass2: レンダリングターゲットをマッピングして表示領域を覆うポリゴンを描画
	 */
	
	// ウィンドウ全体をビューポートにする
	glViewport(0, 0, ww, wh);
	
	// デプステストを無効にする
	glDisable(GL_DEPTH_TEST);
	
	// 通常のフレームバッファへのレンダリング開始
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDrawBuffer(GL_BACK);
	
	// pass2 シェーダーを有効にする
	glUseProgram(pass2);
	
	// pass2 で処理する材質を指定する
	glUniform4fv(reflection, 1, kr);
	glUniform1f(refraction, keta);
	glUniform1f(bgdistance, kbgd);
	
	// テクスチャユニットを指定する
	GLint unitname[TEXNUM];
	for (int i = 0; i < TEXNUM; ++i) {
		unitname[i] = i;
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]);
	}
	glUniform1iv(unit, TEXNUM, unitname);
	
	// 点群の位置を uniform 変数に設定する
	glUniform4fv(point, MAXSAMPLES, pointbuf[0]);
	
	// クリッピング空間いっぱいのポリゴン
	static const GLfloat quad[][2] = {
		{ -1.0f, -1.0f },
		{  1.0f, -1.0f },
		{  1.0f,  1.0f },
		{ -1.0f,  1.0f },
	};
	
	// 表示領域を覆うポリゴンを描く
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof quad / sizeof quad[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// 固定機能シェーダーに戻す
	glUseProgram(0);
	
	// ダブルバッファリング
	glutSwapBuffers();	
	
	ofSetupScreen();
	gui.draw();	
	cout << ofToString(ofGetFrameRate()) << endl;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

	// ウィンドウの幅と高さを覚えておく
	ww = w;
	wh = h;
	
	float halfFov, theTan, screenFov, aspect;
	screenFov 		= 60.0f;
	
	float eyeX 		= (float)w / 2.0;
	float eyeY 		= (float)h / 2.0;
	halfFov 		= PI * screenFov / 360.0;
	theTan 			= tanf(halfFov);
	float dist 		= eyeY / theTan;
	float nearDist 	= dist / 10.0;	// near / far clip plane
	float farDist 	= dist * 10.0;
	aspect 			= (float)w/(float)h;	
	
	// 透視変換行列の設定
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	gluPerspective(fovy, (double)w / (double)h, zNear, zFar);
	gluPerspective(screenFov, aspect, nearDist, farDist);	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, dist, eyeX, eyeY, 0.0, 0.0, 1.0, 0.0);
	
	glScalef(1, -1, 1);           // invert Y axis so increasing Y goes down.
  	glTranslatef(0, -h, 0);       //	
	
	// トラックボールする範囲
	tb->region(w, h);	
	
}

