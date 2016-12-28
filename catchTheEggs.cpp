#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <time.h>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <string.h>

#define ROTTEN 0
#define FRESH 1
using namespace std;

double Deg_Rad = 3.14/180.0;

// Class of a eggs
class Node { 
public:
	double x, y; // x and y coordinates of an egg
	int type; // rotten 0, fresh 1

	Node(double xc,double yc,int typec){
		x=xc;
		y=yc;
		type=typec;
	}
};

// used to draw eggs
void drawEgg(float radiusX, float radiusY,int col) {
if(col==ROTTEN) // change shade for rotten egg
   glColor3f(0.184, 0.310, 0.310);
else	
   glColor3f(1.0, 1.0, 1.0);
   glBegin(GL_TRIANGLE_FAN); // using GL_TRIANGLE_FAN
   glVertex2f(0, 0); // center of circle
   double i;
   int shadeAng = 90;
   int shadeHalfAng = 90;
   for(i = 60; i <=420;i+=0.5) {  // start and end angle of the egg
      float rad = i*Deg_Rad;
      if (i>=240 && i<330) { // used for different shading in this angle
         double color = 1- (i-240)/(3*255);
         glColor3f(color, color, color);
      } else if (i >=330 && i<=420) {
         double color = 1 - (420-i)/(3*255);
         glColor3f(color, color, color);
      } else{
         glColor3f(1.0, 1.0, 1.0);
      }
      glVertex2f(cos(rad)*radiusX, sin(rad)*radiusY);
   }
   glEnd();
}


double xMove=0; // move the egg
double velocity = 0.01; // velocity of egg
int numEggs=4; // number eggs, size of eggList
vector<Node*> *eggList; // list of eggs
vector<double> *yStart; // starting coordinate of each egg
int timer=1;	// timer used to show seconds on screen
double speed=0.03; // increase speed of falling eggs
double minY=-6; // minStart index
double maxYStartVal = 20; //  max depth
double YStartValOffset = 6; // offset of start in Y
double finalLeftLoc = 0; // basket left move
double finalRightLoc =0; // basket
int score = 0; //current score
int highScore=0; // high score 

GLuint texture; // texture on background
 

void FreeTexture( GLuint texture ) { // free Texture
  glDeleteTextures( 1, &texture ); 
}


GLuint LoadTexture( const char * filename, int width, int height ){ // load texture from file, of width and height
	GLuint texture;
	unsigned char * data;
	FILE * file;
	file = fopen(filename,"rb"); 
	data = (unsigned char *)malloc( width * height * 3 );
	fread( data, width * height * 3, 1, file ); // read file
	fclose(file);

	glGenTextures( 1, &texture ); 
	glBindTexture( GL_TEXTURE_2D, texture );  // bind texture to 2d
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ); 

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // wrap texture 
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	free( data ); 
	return texture; 
}


// used for keyPress events
int leftKeyReleased = 0, rightKeyReleased = 0;
void keyPress(int key,int x,int y){
	int i;
	if(key==GLUT_KEY_LEFT){ // left key press
		leftKeyReleased =0;
		finalLeftLoc +=  0.89;
	} else if(key==GLUT_KEY_RIGHT){ // right key press
		rightKeyReleased = 0;
		finalRightLoc += 0.89;
	}
}

// used for key release events
void releaseKey(int key, int x, int y) {
	if(key==GLUT_KEY_LEFT){ // left
		leftKeyReleased =1;
	} else if(key==GLUT_KEY_RIGHT){ // right
		rightKeyReleased = 1;
	}	
}

// to draw an arc which is eventually used to make basket
void DrawCircle(float cx, float cy, double r, double angle,int direction,float zIndex)  { 
	glBegin(GL_LINE_STRIP);
	int max=360;
    for(double i = angle; i < 180-angle; i++) {  /// arc of angle = 180-2*angle 
        float theta = (2.0f * 3.14f* (float)(i) / (float)(max)*(direction));  // 2*pi*r/360
        float x = r * cosf(theta), y = r * sinf(theta); 
        glVertex3f(x + cx, y + cy,zIndex);  // added offset of center of circle
    }
    glEnd(); 
}

// render background image
void renderBackground(){
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);	
	glBindTexture( GL_TEXTURE_2D, texture );
	glBegin (GL_QUADS);
	float a=10, b=5;
	glTexCoord2d(0.0,0.0); 
	glVertex2d(-a,-b);
	glTexCoord2d(1.0,0.0);  
	glVertex2d(+a,-b);
	glTexCoord2d(1.0,1.0);
	glVertex2d(+a,+b);
	glTexCoord2d(0.0,1.0);
	glVertex2d(-a,+b);
	glEnd();
}


// increase speed and timer and  check if eggs have moved out of screen
void idle(){	
	if(timer%1800==0) // increase speed of eggs
		speed+=0.005;

	timer++;
	for(int j=0;j<numEggs;j++){
		if((eggList->at(j))->y < minY){
			(eggList->at(j))->x = -7+rand()%15;
			(eggList->at(j))->y = YStartValOffset+maxYStartVal*((double)rand()/(RAND_MAX));
		} else{
			(eggList->at(j))->y = (eggList->at(j))->y-speed;	
		}
	}
	glutPostRedisplay();
}

double basketVel = 20;// basket velocity
double basketTimer = 0.002; // basket timer
double basketInitX = 0; // init coordinates of basket
double eggRadiusX = 0.4, eggRadiusY = 0.58; // X and Y radius of eggs

int gameTime = 0; // timer in the eggs appearing on top left

void display(void) {
	gameTime = 30 -timer/200;
	if(gameTime <=0){
		gameTime = 0;
	}

	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	glPushMatrix(); // background image
		glEnable( GL_TEXTURE_2D );
		renderBackground();	
		glDisable( GL_TEXTURE_2D );
	glPopMatrix();	


	glPushMatrix(); // high score background 2
		glTranslatef(6.4,4.15,0);	
		glColor3f(1,1,1);
		glScalef(4.5,0.7,1);
		glutSolidCube(1);		
	glPopMatrix();
	glPushMatrix(); // high score background
		glTranslatef(6.45,4.18,0);	
		glColor3f(0.545, 0.271, 0.075);
		glScalef(4.5,0.7,1);
		glutSolidCube(1);		
	glPopMatrix();
	glPushMatrix(); // high score text		
		string a1 = "Highscore:";
		stringstream s11;
	    s11 << highScore;
	    string ans1 =  s11.str();
	    ans1 = a1+ans1; // create text
	    char const *str1 = ans1.c_str();
		glColor3f(1, 1, 1.000); // color
		glTranslatef(4.5,4.05,0); // translate
		glScalef(0.003,0.003,1); // scale

		for (int j=0;j<strlen(str1);j++) {
			char c1 = str1[j];
			glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN , c1);// select stroke style
		}
	glPopMatrix();


	glPushMatrix(); // score background 2
		glTranslatef(6.75,3.25,0);	
		glColor3f(1, 1, 1);
		glScalef(3.5,0.7,1);
		glutSolidCube(1);		
	glPopMatrix();
	glPushMatrix(); // score background
		glTranslatef(6.8,3.3,0);	
		glColor3f(0.824, 0.412, 0.118);
		glScalef(3.5,0.7,1);
		glutSolidCube(1);		
	glPopMatrix();

	string scoreText = "Score:"; // to print score on screen
	stringstream strS;
    strS << score;
    string final =  strS.str(); // add score to score text
    final = scoreText+final;
    char const *text = final.c_str(); // create final text

    glPushMatrix(); // high score text		
		glColor3f(1, 1, 1);
		glTranslatef(5.8,3.2,0);
		glScalef(0.003,0.003,1);
		for (int j=0;j<strlen(text);j++) {
			char c1 = text[j];
			glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN , c1);
		}
	glPopMatrix();

	glPushMatrix(); // timer background
		glTranslatef(-6.96,4.12,0);
		glColor3f(1.000, 1.000, 1.000);
		glScalef(4.5,0.7,1);
		// glScalef(7,0.7,1);
		glutSolidCube(1);
	glPopMatrix();

	glPushMatrix(); // timer background 2
		glTranslatef(-7,4.15,0);
		glColor3f(1.000, 0.647, 0.000);
		glScalef(4.6,0.7,1);
		glutSolidCube(1);
	glPopMatrix();

	// time left rendering
	string a = "Time left:";
	stringstream s1;
    s1 << gameTime;
    string ans =  s1.str();
    ans = a+ans;
    char const *str = ans.c_str();

	glPushMatrix(); // time text
		glColor3f(1,1,1);
		glTranslatef(-8.52,4.02,0);
		glScalef(0.003,0.003,1);
		
		for (int j=0;j<strlen(str);j++) {
			char c = str[j];
			glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN , c);
		}
	glPopMatrix();

	// render eggs
	glPushMatrix();
	if (leftKeyReleased ==1){ // used to stop basket from moving in left dir
		finalLeftLoc =0;
		leftKeyReleased =0;
	}
	if (rightKeyReleased == 1) { // used to stop basket from moving in right dir
		finalRightLoc =0;
		rightKeyReleased = 0;
	}

	// used in moving the basket left or right
	if (finalLeftLoc >0) { 
		basketInitX -= basketTimer*basketVel;
		glTranslatef(basketInitX ,(GLfloat)-2.5,0);	
		finalLeftLoc -= basketTimer*basketVel;
	} else if (finalRightLoc > 0) {
		basketInitX += basketTimer*basketVel;
		glTranslatef(basketInitX,(GLfloat)-2.5,0);	
		finalRightLoc -= basketTimer*basketVel;
	} else {
		glTranslatef(basketInitX,(GLfloat)-2.5,0);
	}
	glScalef(0.53,0.53,1);
	
	double i,j,rad = 0;

	// draws back side of basket
	for(j=1;j<=4; j+=0.02){
		double rad = 1 + j/20;
		int val = (int)(j*100);
		if (val%10 < 5 ) {
			glColor3f(0.855, 0.647, 0.125);
		} else {
			glColor3f(0.627, 0.322, 0.176);
		}  
		double centerY=-3.8;
		DrawCircle(0, centerY, j,39+j,1,0);
	}

	// draws front side of basket
	for(i=45;i<=80;i+=0.1) {
		double rad = 2 + i/20;
		int val = (int)(i*10);
		double t = (double)(i-45)/35;
		if (i < 49) {
			glColor3f(0.545, 0.271, 0.075);
		} else {
			glColor3f(0.545*(t) + 0.855*(1-t), 0.271*t + 0.647*(1-t), 0.075*t + 0.125*(1-t));
		}
		DrawCircle(0, 2, rad,i,-1,-1);
	}

	// ribbon on the basket
	double centerY = 0.5;
	i = 80;  
	glScalef(1,0.8,1);
	for(j=0;j<100;j++) {
		double rad = j/20;
		glColor3f(1.000, 1, 0.000);
		DrawCircle(0, centerY - rad, 4- rad , i-rad*20 ,-1,-1);
	}
	glPopMatrix();
	
	if(gameTime>0) {
		// render eggs
		for(int k=0;k<numEggs;k++){
			glPushMatrix();
			    glTranslatef((eggList->at(k))->x, (eggList->at(k))->y , 0);
			    double eggX = (eggList->at(k))->x, eggY = (eggList->at(k))->y; // egg coordinate
			    double widL = 1.4, widR = 1.4, height = -2.8;
			    if (eggX - eggRadiusX > basketInitX - widL &&  eggX + eggRadiusX< basketInitX + widR) {
			    	if (eggY+eggRadiusY < height  ) {			    		
			    		if (eggList->at(k)->type == ROTTEN) { // if rotten eggs is caugth reduce score
			    			score--;
			    		} else {
			    			score++;
			    		}
			    		
			    		(eggList->at(k))->x = -7+rand()%15; // select new position of egg
						(eggList->at(k))->y = YStartValOffset+maxYStartVal*((double)rand()/(RAND_MAX));
			    		if(rand()%numEggs==1) // select type
			    			(eggList->at(k))->type = ROTTEN;
			    		else
			    			(eggList->at(k))->type = FRESH;	
			    	
			    	}
			    }
			  
			    if (eggY + eggRadiusY < -4.8){ // egg is off the screen
			    	(eggList->at(k))->x = -8+rand()%17; // new X Coordinate
					(eggList->at(k))->y = YStartValOffset+maxYStartVal*((double)rand()/(RAND_MAX)); // new Y coordinate 
			    	if(rand()%numEggs==1) // type of egg
		    			(eggList->at(k))->type = ROTTEN;
		    		else
		    			(eggList->at(k))->type = FRESH;
			    }
	    		
			    drawEgg(eggRadiusX, eggRadiusY,eggList->at(k)->type);// draw egg
			glPopMatrix();
		}
	} else {
		
		glPushMatrix();		// game over text's: background 
			glTranslatef(0,1,0);
			glColor3f(1.000, 1.000, 0.000);
			glScalef(4,0.7,1);
			glutSolidCube(1);
		glPopMatrix();

		glPushMatrix(); // game over text
			string a = "Game Over";
			stringstream s1;
		    s1 << score;
		    char const *str = a.c_str();// create text
			glColor3f(0.000, 0.392, 0.000);
			glPushMatrix();
			
			glTranslatef(-1.3,0.9,0);
			glScalef(0.003,0.003,1);

			for (int j=0;j<strlen(str);j++) {
				char c = str[j];
				glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN , c); // stroke style
			}
			
		glPopMatrix();


		if(score > highScore){ // if higher than high score
			FILE* fd = fopen("Highscore.txt","w+"); // write to file
			fprintf(fd,"%d",score);
			// highScore=score;
			fclose(fd);
			
			glPushMatrix();		// high score broken background 
				glTranslatef(0.2,0.1,0);
				glColor3f(0.545, 0.271, 0.075);
				glScalef(8.6,0.7,1);
				glutSolidCube(1);
			glPopMatrix();

			glPushMatrix();
				string a = "You Broke the High Score!!"; //  write this text 
				stringstream s1;
			    s1 << score;
			    char const *str = a.c_str();
				glColor3f(1.000, 1, 1.000);
				glPushMatrix();
				
				glTranslatef(-3.8,0,0);
				glScalef(0.003,0.003,1);

				for (int j=0;j<strlen(str);j++) {
					char c = str[j];
					glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN , c);
				}
			glPopMatrix();
		}		
	}
	
	glFlush();
}
	
// reshape function passed with width and height	
void reshape(int w, int h) {
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (w <= h)
      glOrtho(-5.0, 5.0, -5.0*(GLfloat)h/(GLfloat)w, 
               5.0*(GLfloat)h/(GLfloat)w, -5.0, 5.0);
   else
      glOrtho(-5.0*(GLfloat)w/(GLfloat)h, 
               5.0*(GLfloat)w/(GLfloat)h, -5.0, 5.0, -5.0, 5.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

int main(int argc, char** argv)
{	

//  read the high score from file
	FILE* fd = fopen("Highscore.txt","r+");
	fscanf(fd,"%d",&highScore);
	fclose(fd);

	srand(time(NULL));		
	
	eggList=new vector<Node*>(); // list of eggs
	
	double i;
	double xEgg;
	double yEgg;
	for(i=0;i<numEggs;i++){ // initialize eggs x , y and type
		xEgg = -8+rand()%17;
		yEgg= YStartValOffset+ maxYStartVal*((double)rand()/(RAND_MAX));
		Node* egg=NULL;
		if(rand()%numEggs==1) {
			egg=new Node(xEgg,yEgg,ROTTEN);
		} else {
			egg=new Node(xEgg,yEgg,FRESH);
		}
		eggList->push_back(egg);
	}


	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_RGBA | GLUT_ALPHA);
	glutInitWindowSize (1024, 600);
	glutInitWindowPosition (0, 0);
	glutCreateWindow ("Catch The Eggs");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutSpecialFunc(keyPress);
	glutSpecialUpFunc(releaseKey);
	// glutMainLoop();

	texture = LoadTexture("back.raw", 960, 600 ); // load the background 
	glutMainLoop();
	FreeTexture( texture );
	return 0;
}
