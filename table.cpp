#include <GL/glut.h>
#include <Windows.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include <math.h>
#include <cmath>

#define MAX_NUM_PARTICLES 1000
#define INITIAL_NUM_PARTICLES 25
#define INITIAL_POINT_SIZE 5.0
#define INITIAL_SPEED 1.0
GLfloat angle=10.0f;
typedef int BOOL;
#define TRUE 1
#define FALSE 0

void myDisplay();
void myIdle();
void myReshape(int, int);
void collision(int);
float forces(int, int);

/* particle struct */
typedef struct particle {
    int color;                
    float position[3];        
    float velocity[3];        
    float mass;               
} particle;

particle particles[MAX_NUM_PARTICLES]; 

int present_time;
int last_time;
int num_particles = INITIAL_NUM_PARTICLES;  
float point_size = INITIAL_POINT_SIZE;      
float speed = INITIAL_SPEED;                
bool gravity = TRUE;                       
bool elastic = TRUE;                       
bool repulsion = FALSE;                     
float coef = 1.0;                           
float d2[MAX_NUM_PARTICLES][MAX_NUM_PARTICLES]; 
GLsizei wh = 500, ww = 500; 

void polygon(int a, int b, int c , int d)
{
	GLfloat vertices[][3] = {{-1.0,-1.0,-1.0},{1.0,-1.0,-1.0},
	{1.0,1.0,-1.0}, {-1.0,1.0,-1.0}, {-1.0,-1.0,1.0}, 
	{1.0,-1.0,1.0}, {1.0,1.0,1.0}, {-1.0,1.0,1.0}};
	
	GLfloat normals[][3] = {{-1.0,-1.0,-1.0},{1.0,-1.0,-1.0},
	{1.0,1.0,-1.0}, {-1.0,1.0,-1.0}, {-1.0,-1.0,1.0}, 
	{1.0,-1.0,1.0}, {1.0,1.0,1.0}, {-1.0,1.0,1.0}};
	
	GLfloat colors[][4] = {{1.0,1.0,1.0,0.5},{1.0,0,1.0,0.5},
	{1.0,1.0,1.0,0.5}, {1.0,1.0,1.0,0.5}, {1.0,1.0,1.0,0.5}, 
	{1.0,1.0,1.0,0.5}, {1.0,1.0,1.0,0.5}, {1.0,1.0,1.0,0.5}};
/* draw a polygon via list of vertices */

 	glBegin(GL_POLYGON);
    glColor4fv(colors[a]);  
    glTexCoord2f(0.0,0.0);  //配置紋理坐標
    glVertex3fv(vertices[a]);  
    glColor4fv(colors[b]);  
    glTexCoord2f(1.0,0.0);
    glVertex3fv(vertices[b]);
    glColor4fv(colors[c]);
    glTexCoord2f(1.0,1.0);
    glVertex3fv(vertices[c]);
    glColor4fv(colors[d]);
    glTexCoord2f(0.0,1.0);
    glVertex3fv(vertices[d]);
	glEnd();
}																							

void colorcube(void)
{

/* map vertices to faces */

	polygon(0,0,0,3);
	polygon(0,0,2,3);
	polygon(0,4,0,0);
	polygon(1,2,0,0);
	polygon(4,5,6,7);
	polygon(0,1,5,4);
}	


GLfloat colors[8][3] = { {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}, {0.0, 1.0, 1.0}, {1.0, 0.0, 1.0}, {1.0, 1.0, 0.0},
    {1.0, 1.0, 1.0} };

void InitEnvironment()
{
	//glColor4f(0,0,1,1);
	glEnable(GL_DEPTH);
	//glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65,1,1,50);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(12,12,20,0,0,0,0,1,0);
	
	int  i, j;
    for (i = 0; i < num_particles; i++) { 
        particles[i].mass = 1.0;
        particles[i].color = i % 8;
        for (j = 0; j < 3; j++) {
            particles[i].position[j] = 2.0 * ((float)rand() / RAND_MAX) - 1.0;
            particles[i].velocity[j] = speed * 2.0 * ((float)rand() / RAND_MAX) - 1.0;
        }
    }
    glPointSize(point_size);

    glClearColor(0.5, 0.0, 1.0, 1.0); 
}

void myReshape(int w, int h) {
    glViewport(0, 0, w, h);

	/* Use a perspective view */
	
 	glMatrixMode(GL_PROJECTION); 
 	glLoadIdentity();
	if(w<=h) 
		glFrustum(-2.0, 2.0, -2.0 * (GLfloat) h/ (GLfloat) w, 2.0* (GLfloat) h / (GLfloat) w, 2.0, 25.0);
		
	else glFrustum(-2.0, 2.0, -2.0 * (GLfloat) w/ (GLfloat) h, 
       2.0* (GLfloat) w / (GLfloat) h, 2.0, 20.0);

 	glMatrixMode(GL_MODELVIEW);
 	/*if (w <= h)       //寬比高等於或較長 
        glOrtho(-2.0, 2.0, -2.0 * (GLfloat) h / (GLfloat) w,   //通過正交投影把模型按照1:1的比例繪制到剪裁面上  參數:(左，右，上，下)
            2.0 * (GLfloat) h / (GLfloat) w, 10.0, 40.0);
    else
        glOrtho(-2.0 * (GLfloat) w / (GLfloat) h,
            2.0 * (GLfloat) w / (GLfloat) h, -2.0, 2.0, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);   */ 
}

void myIdle() {
    int i, j, k;
    float dt;
    present_time = glutGet(GLUT_ELAPSED_TIME);
    dt = 0.001 * (present_time - last_time); 
    for (i = 0; i < num_particles; i++) {
        for (j = 0; j < 3; j++) {
            particles[i].position[j] += dt * particles[i].velocity[j];
            particles[i].velocity[j] += dt * forces(i, j) / particles[i].mass;
        }
        collision(i);
    }
    if (repulsion)
        for (i = 0; i < num_particles; i++)
            for (k = 0; k < i; k++) {
                d2[i][k] = 0.0;
                for (j = 0; j < 3; j++)
                    d2[i][k] += (particles[i].position[j] - particles[k].position[j]) *
                    (particles[i].position[j] - particles[k].position[j]);
                d2[k][i] = d2[i][k];
            }
    last_time = present_time;
    glutPostRedisplay();
}

float forces(int i, int j) { 	//重力 
    int k;
    float force = 0.0;
    if (gravity && j == 1)
        force = -1.0; 
    if (repulsion)
        for (k = 0; k < num_particles; k++) { 
            if (k != i)
                force += 0.001 * (particles[i].position[j] - particles[k].position[j]) / (0.001 + d2[i][k]);
        }
    return(force);
}

void collision(int n) {		//計算碰撞 
    
    int i;
    for (i = 0; i < 3; i++) {
        if (particles[n].position[i] >= 1.0) {
            particles[n].velocity[i] = -coef * particles[n].velocity[i];
            particles[n].position[i] = 1.0 - coef * (particles[n].position[i] - 1.0);
        }
        if (particles[n].position[i] <= -1.0) {
            particles[n].velocity[i] = -coef * particles[n].velocity[i];
            particles[n].position[i] = -1.0 - coef * (particles[n].position[i] + 1.0);
        }
    }
}

class Camera {
	public:  
	    double theta;      //position of x and z
	    double y;          //y
	    double dTheta;     //angle change
	    double dy;         //y change
	public:
	    //init
	    Camera() : theta(0), y(3), dTheta(0.04), dy(0.2) {}
	   
	    double getX() { return 1+5 * cos(theta); }
	    double getY() { return y; }
	    double getZ() { return 1+5 * sin(theta); }
	    void moveRight() { theta += dTheta; }
	    void moveLeft() { theta -= dTheta; }
	    void moveUp() { y += dy; }
	    void moveDown() { y -= dy; }
};

Camera camera;

void display(void)
{
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(camera.getX(), camera.getY(), camera.getZ(), 1.0, 1.0, 1.0, 0.0, 1.0, 0.0);
	int i;
    glBegin(GL_POINTS); /* render all particles */
    for (i = 0; i < num_particles; i++) {
        glColor3fv(colors[particles[i].color]);
        glVertex3fv(particles[i].position);
    }
    glEnd();
    colorcube();
    
 	glFlush();
	glutSwapBuffers();
}

void onKey(int key, int , int )
{

/* Use x, X, y, Y, z, and Z keys to move viewer */

   switch (key) {
    case GLUT_KEY_LEFT: camera.moveLeft(); break;
    case GLUT_KEY_RIGHT: camera.moveRight(); break;
    case GLUT_KEY_UP: camera.moveUp(); break;
    case GLUT_KEY_DOWN: camera.moveDown(); break;
    }
   display();
}

void KeyBoards(unsigned char key,int x,int y)
{
	switch (key)
	{
	case 'w':
		glMatrixMode(GL_MODELVIEW);
		glRotatef(angle,0,0,-1);
		glutPostRedisplay();
		break;
	case 'a':
		glMatrixMode(GL_MODELVIEW);
		glRotatef(angle,-1,0,0);
		glutPostRedisplay();
		break;
	case 's':
		glMatrixMode(GL_MODELVIEW);
		glRotatef(angle,0,0,1);
		glutPostRedisplay();
		break;
	case 'd':
		glMatrixMode(GL_MODELVIEW);
		glRotatef(angle,1,0,0);
		glutPostRedisplay();
		break;
	case 'z':
		glMatrixMode(GL_MODELVIEW);
		glRotatef(angle,0,1,0);
		glutPostRedisplay();
		break;
	case 'x':
		glMatrixMode(GL_MODELVIEW);
		glRotatef(angle,0,-1,0);
		glutPostRedisplay();
		break;

	case 27:
		exit(0);
		break;
	}
}

void myDisplay() { 
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glLoadIdentity();
	int i;
    glBegin(GL_POINTS); /* render all particles */
    for (i = 0; i < num_particles; i++) {
        glColor3fv(colors[particles[i].color]);
        glVertex3fv(particles[i].position);
    }
    glEnd();
 	colorcube();

 	glFlush();
	glutSwapBuffers();
}



int main(int argc, char** argv)
{
    GLubyte image[4][4][3];     //宣告相關數值 
	int i, j, c;
	for(i=0;i<4;i++)
	{
	 for(j=0;j<4;j++)                     //將 image陣列裡面的所有值畫成 2*2方格棋盤(每個方個由 2*2個像點組成) 
	 {
	   c = ((((i&0x2)==0)^((j&0x2))==0))*255;
	   image[i][j][0]= (GLubyte) c;
	   image[i][j][1]= (GLubyte) c;
	   image[i][j][2]= (GLubyte) c;
	 }
	}
	glutInit(&argc, argv);   //初始化GLUT
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);  
	glutInitWindowPosition(500, 200);    
	glutInitWindowSize(500, 500);    
	glutCreateWindow("table"); 
	InitEnvironment();   //初始化顯示環境
	glutKeyboardFunc(&KeyBoards);  //註冊鍵盤事件
	glutDisplayFunc(&myDisplay);   //回撥函式 
	
	glutReshapeFunc(myReshape);
 	//glutDisplayFunc(display);
 	glutSpecialFunc(onKey);

	glEnable(GL_DEPTH_TEST);     //啟用深度 
    glEnable(GL_TEXTURE_2D);     //啟用 2維紋理 
    glTexImage2D(GL_TEXTURE_2D,0,3,4,4,0,GL_RGB,GL_UNSIGNED_BYTE, image); //指定的參數生成一個2D紋理RGB 4*4的大小
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);   //設置紋理相關參數(2維、以x軸延伸、重複紋理)
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);   //(2維、以y軸延伸、重複紋理)
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);  //(2維、設置最大過濾、最接近的一個像素的顏色作為繪製像素的顏色)
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
	glEnable(GL_DEPTH_TEST);
	
	glutIdleFunc(myIdle);
	glutMainLoop();    //持續顯示，當視窗改變會重新繪製圖形
	return 0;    

}
