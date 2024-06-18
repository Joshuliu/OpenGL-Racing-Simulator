/******************************************
*
* Author: Joshua Liu
* Environment/Compiler: Xcode 15.2
*
* Movement Controls
*  W: Accelerate the vehicle. Speed increases until it reaches a maximum limit.
*  S: Decelerate or reverse the vehicle. Speed decreases, and the vehicle can move backward up to a maximum reverse speed.
*  A: Steer left. Turns the vehicle left and adjusts the wheel angle accordingly, up to a maximum angle.
*  D: Steer right. Turns the vehicle right and adjusts the wheel angle accordingly, down to a minimum angle.
* General Controls
*  ESC: Exit the game.
*  R: Reset the game state, repositioning the player's vehicle to the starting point and resetting lap timing and vehicle velocity.
* Camera and View Controls
*  C: Toggle rear view (look behind).
*  Q: Look left without changing the direction of movement.
*  E: Look right without changing the direction of movement.
*  X: Toggle between first-person view (FPV) and third-person view.
* Miscellaneous Controls
*  H: Cycle through different headlight modes: Auto, Off, Low, High
* Stepping Controls (stepping through walls)
*  Arrow Up: Move forward relative to the vehicle’s current direction.
*  Arrow Down: Move backward relative to the vehicle’s current direction.
*  Arrow Left: Rotate the vehicle’s view left without moving.
*  Arrow Right: Rotate the vehicle’s view right without moving.
* Mouse Controls
*  Left click start window to select options and start game.
*  Right click gameplay window to use popup menu and change day/night.
*
*******************************************/

#include <iostream>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <map>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159
#define MAX_CONFETTI 100  // Number of confetti particles
using namespace std;

/*\ ---- Global Variables ---- \*/

// Gameplay mechanics and vehicle dynamics
bool gameStarted = false;
float cameraAngle = 0.0f; // Angle for the circular camera motion
static float meX = 240, meY = 0, meZ = -40;
float velocity = 0.0, acceleration = 0.05, deceleration = 0.02, maxVelocity = 3.0;
static float angleX = 0, angleY = -1;
float turnSpeed = 3.0, wheelAngle = 0;
static float stepsize = 5.0, turnsize = 10.0;  // Navigation clipping
int currentCheckpoint = 0;
bool timerRunning = false;
float currentLapTime = 0;
int lapStartTime = 0, lapEndTime = 0;

// Environment settings
int mainWindow, startWindow;
float globAmb[] = { 0.125, 0.125, 0.125, 1.0 };
bool day = true;

// Visual and interaction enhancements
bool lookBehind = false, lookLeft = false, lookRight = false;
bool fpv = false;  // First-person view toggle
bool useIdleFunc = false;  // Toggle to avoid idle function lag
int headlightMode = 3; // Headlight settings: 0 = off, 1 = low beam, 2 = high beam, 3 = auto low beam
std::map<char, bool> keyStates;  // Keyboard state tracking

// Lighting and animation timings
int currentLightRow = -1;  // Start before the first row (-1)
int lightUpdateTime = 1000;  // Time in milliseconds between lights
unsigned long lastUpdateTime = 0;  // Last update timestamp

// Decorative elements
bool isDaySelected = true;
bool isNightSelected = false;
static float teapotRotationAngle = 0.0f; // For rotating objects like teapots
GLuint textureGrass[1];
struct BitMapFile {
    int sizeX;
    int sizeY;
    unsigned char *data;
};
float cloudSpeed = 0.05;
struct CloudPosition {
    float x, y, z;
};
struct CloudPart {
    float offsetX, offsetY, offsetZ, size;
};
struct Tree {
    float x, y, z, trunkHeight, treeHeight;
    Tree(float x, float y, float z, float trunkHeight, float treeHeight)
        : x(x), y(y), z(z), trunkHeight(trunkHeight), treeHeight(treeHeight) {}
};
struct ConfettiParticle {
    float position[3];
    float velocity[3];
    float color[3];
    bool active;
};
ConfettiParticle confettiCannon1[MAX_CONFETTI];
ConfettiParticle confettiCannon2[MAX_CONFETTI];

/*\ -------------------------- \*/

/*\ --- Coordinate Arrays ---- \*/
int axisBarriersCount = 18;
float axisBarriers[][6] = { // Track along-axis barrier coordinates
    {-75, 0, -320, -200, 2.5, -325},
    {-80, 0, -112.5, -75, 2.5, -320},
    {-200, 0, -325, -205, 2.5, 200},
    {-205, 0, 200, -40, 2.5, 205},
    {-205, 0, 275, -35, 2.5, 280},
    {-275, 0, 200, -280, 2.5, -395},
    {2.5, 0, -55, -2.5, 2.5, -395},
    {80, 0, -112.5, 75, 2.5, -317.5},
    {125, 0, 200, 120, 2.5, 280},
    {160, 0, -235, 200, 2.5, -240},
    {160, 0, -240, 240, 2.5, -245},
    {160, 0, -160, 205, 2.5, -165},
    {200, 0, 195, 195, 2.5, 280},
    {200, 0, -160, 205, 2.5, 125},
    {280, 0, -160, 275, 2.5, 120},
    {240, 0, -400, -280, 2.5, -395},
    {240, 0, -322.5, 75, 2.5, -317.5},
    {240, 0, -322.5, 75, 2.5, -317.5},
};
int curveBarriersCount = 13;
float curveBarriers[][7] = { // Track curved barrier coordinates
    {80, 2.5, 280, 120, 115, 0, PI},
    {80, 2.5, 280, 45, 40, 0, PI},
    {-40, 2.5, 280, 80, 75, 3 * PI / 2, 2 * PI},
    {-200, 2.5, 200, 80, 75, PI / 2, PI},
    {240, 2.5, -320, 80, 75, 3 * PI / 2, 5 * PI / 2},
    {160, 2.5, -240, 80, 75, PI / 2, PI},
    {200, 2.5, -160, 80, 75, 3 * PI / 2, 2 * PI},
    {0, 2.5, 0, 140, 135, 1.6875 * PI, 3.3125 * PI},
    {0, 2.5, 0, 60, 55, 1.5125 * PI, 3.4875 * PI},
    {200, 2.5, 120, 80, 75, 0, PI / 2},
    {200, 2.5, 200, 80, 75, PI, 3 * PI / 2},
    {80, 2.5, -240, 5, 0, 3 * PI / 2, 2 * PI},
    {160, 2.5, -240, 5, 0, PI / 2, 3 * PI / 2},
};
float trackQuads[][5][3] = { // Track quad coordinates
    {{280, 0, -160}, {280, 0, 120}, {200, 0, 120}, {200, 0, -160}, {0, 1, 0}},
    {{200, 0, 200}, {200, 0, 280}, {120, 0, 280}, {120, 0, 200}, {0, 1, 0}},
    {{280, 0, -160}, {280, 0, 120}, {200, 0, 120}, {200, 0, -160}, {0, 1, 0}},
    {{-40, 0, 200}, {-40, 0, 280}, {-200, 0, 280}, {-200, 0, 200}, {0, 1, 0}},
    {{-200, 0, 200}, {-280, 0, 200}, {-280, 0, -400}, {-200, 0, -400}, {0, 1, 0}},
    {{-80, 0, -400}, {-80, 0, -320}, {-200, 0, -320}, {-200, 0, -400}, {0, 1, 0}},
    {{240, 0, -400}, {240, 0, -240}, {-80, 0, -240}, {-80, 0, -400}, {0, 1, 0}},
    {{200, 0, -240}, {200, 0, -160}, {160, 0, -160}, {160, 0, -240}, {0, 1, 0}},
    {{-80, 0, -240}, {-80, 0, -120}, {80, 0, -120}, {80, 0, -240}, {0, 1, 0}},
};
float trackCurves[][7] = { // Track curve coordinates
    {80, 0, 280, 120, 40, 0, PI},
    {-40, 0, 280, 80, 0, 3 * PI / 2, 2 * PI},
    {-200, 0, 200, 80, 0, PI / 2, PI},
    {240, 0, -320, 80, 0, 3 * PI / 2, 5 * PI / 2},
    {160, 0, -240, 80, 0, PI / 2, PI},
    {200, 0, -160, 80, 0, 3 * PI / 2, 2 * PI},
    {0, 0, 0, 140, 60, 0, 2 * PI},
    {200, 0, 120, 80, 0, 0, PI / 2},
    {200, 0, 200, 80, 0, PI, 3 * PI / 2},
};
float slantedQuads[][5][3] = { // Racecar quads coordinates
    {{-5, 5, 45}, {5, 5, 45}, {5, 10, 15}, {-5, 10, 15}, {0, 1, 0.25}}, // Front wing support
    {{-5, 15, -15}, {5, 15, -15}, {5, 10, -25}, {-5, 10, -25}, {0, 1, -0.5}}, // Behind cockpit slant
    {{5, 0, 15}, {5, 10, 15}, {15, 10, 5}, {15, 0, 5}, {1, 0, 0.5}}, // Body front left
    {{-5, 0, 15}, {-5, 10, 15}, {-15, 10, 5}, {-15, 0, 5}, {-1, 0, 0.5}}, // Body front right
    {{15, 0, -15}, {15, 10, -15}, {5, 10, -25}, {5, 0, -25}, {1, 0, -0.5}}, // Body back left
    {{-15, 0, -15}, {-15, 10, -15}, {-5, 10, -25}, {-5, 0, -25}, {-1, 0, -0.5}}, // Body back right
    {{-5, 10, -25}, {5, 10, -25}, {5, 15, -35}, {-5, 15, -35}, {0, 1, 0.5}}, // Rear wing support front
    {{-5, 10, -35}, {5, 10, -35}, {5, 15, -40}, {-5, 15, -40}, {0, 1, 0.5}}, // Rear wing support back
    {{-5, 10, -25}, {-5, 15, -35}, {-5, 15, -40}, {-5, 10, -35}, {-1, 0, 0}}, // Rear wing support right
    {{5, 10, -25}, {5, 15, -35}, {5, 15, -40}, {5, 10, -35}, {1, 0, 0}}, // Rear wing support left
};
float slantedTriangles[][4][3] = { // Racecar triangles coordinates
    {{-5, 10, 15}, {-5, 10, 5}, {-15, 10, 5}, {0, 1, 0}}, // Body front left
    {{5, 10, 15}, {5, 10, 5}, {15, 10, 5}, {0, 1, 0}}, // Body front right
    {{-5, 15, -15}, {-5, 10, -25}, {-15, 10, -15}, {-0.5, 1, -0.5}}, // Body back left
    {{5, 15, -15}, {5, 10, -25}, {15, 10, -15}, {0.5, 1, -0.5}}, // Body back right
    {{-5, 5, 45}, {-5, 5, 15}, {-5, 10, 15}, {-1, 0, 0}}, // Front wing support left
    {{5, 5, 45}, {5, 5, 15}, {5, 10, 15}, {1, 0, 0}}, // Front wing support right
};
CloudPosition cloudPositions[] = {
    {5.0, 5.0, 10.0},
    {-15.0, 7.0, 5.0},
    {10.0, 6.0, -10.0},
    {-20.0, 8.0, -5.0},
    {15.0, 5.0, 15.0},
    {-25.0, 7.0, 20.0}
};
CloudPart cloudParts[] = {
    {0.0, 0.0, 0.0, 0.5}, // Central sphere
    {0.6, 0.0, 0.3, 0.3}, // Right-front sphere
    {-0.6, 0.0, 0.3, 0.4}, // Left-front sphere
    {1.2, 0.0, -0.3, 0.3}, // Far right sphere
    {-1.2, 0.0, -0.3, 0.35}, // Far left sphere
    {0.0, 0.0, -0.6, 0.4}, // Back central sphere
    {0.6, 0.0, -0.9, 0.25}, // Back right sphere
    {-0.6, 0.0, -0.9, 0.3} // Back left sphere
};
std::vector<Tree> trees = {
    {253, 0, 196, 5.6, 10.3},
    {230, 0, 326, 6.7, 9.5},
    {174, 0, 87, 7.3, 12.3},
    {160, 0, 120, 6.2, 10.7},
    {156, 0, -50, 7.1, 12.6},
    {80, 0, 280, 8.4, 12.5},
    {10, 0, 180, 9.5, 14.3},
    {-90, 8, 360, 5.6, 13.1},
    {-140, 0, 310, 7, 12},
    {-160, 0, 160, 5.6, 13.1},
    {-150, 0, 36, 7.2, 12.2},
    {-185, 0, -86, 7.1, 12.5},
    {-150, 0, -175, 7.8, 12.3}
};
GLfloat hillPoints[4][4][3] = {
    {{-1.5, 0.0, -1.5}, {-0.5, 0.0, -1.5}, {0.5, 0.0, -1.5}, {1.5, 0.0, -1.5}},
    {{-1.5, 0.0, -0.5}, {-0.5, 1.0, -0.5}, {0.5, 1.0, -0.5}, {1.5, 0.0, -0.5}},
    {{-1.5, 0.0, 0.5}, {-0.5, 1.0, 0.5}, {0.5, 1.0, 0.5}, {1.5, 0.0, 0.5}},
    {{-1.5, 0.0, 1.5}, {-0.5, 0.0, 1.5}, {0.5, 0.0, 1.5}, {1.5, 0.0, 1.5}}
};
/*\ -------------------------- \*/


/*\ ---- Helper Functions ---- \*/
// Load bmp files
BitMapFile *getBMPData(string filename) {
    BitMapFile *bmp = new BitMapFile;
    unsigned int size, offset, headerSize;
    ifstream infile(filename.c_str(), ios::binary);
    infile.seekg(10);
    infile.read((char *) &offset, 4);
    infile.read((char *) &headerSize, 4);
    infile.seekg(18);
    infile.read((char *) &bmp->sizeX, 4);
    infile.read((char *) &bmp->sizeY, 4);
    size = bmp->sizeX * bmp->sizeY * 24;
    bmp->data = new unsigned char[size];
    infile.seekg(offset);
    infile.read((char *) bmp->data, size);
    for (int i = 0; i < size; i += 3) {
        char temp = bmp->data[i];
        bmp->data[i] = bmp->data[i+2];
        bmp->data[i+2] = temp;
    }
    return bmp;
}
// Load grass texture
void loadGrassTexture() {
    BitMapFile *image[1];
    image[0] = getBMPData("textures/smallgrass.bmp");
    
    glGenTextures(1, textureGrass);
    glBindTexture(GL_TEXTURE_2D, textureGrass[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[0]->sizeX, image[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image[0]->data);

    // Error check
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(err) << std::endl;
    }
}
// Generate a random float in range [min, max]
float randomFloatInRange(float min, float max) {
    return min + ((float)rand() / (float)(RAND_MAX)) * (max - min);
}
// Routine to draw a bitmap character string.
void drawText(const char* string, int x, int y) {
    glDisable(GL_LIGHTING);
    glColor3f(!day, !day, !day); // Set text color
    glRasterPos2i(x, y); // Position the text correctly
    while (*string) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *string++);
    }
    glEnable(GL_LIGHTING);
}
void renderCenteredText(const char* string) {
    glDisable(GL_LIGHTING);
    glColor3f(!day, !day, !day); // Set text color
    int x = glutGet(GLUT_WINDOW_WIDTH) / 2 - strlen(string) * 4.5; // Approximate center
    int y = glutGet(GLUT_WINDOW_HEIGHT) / 2 - 250;

    // Position the text in the middle of the screen
    glRasterPos2i(x, y);

    // Loop through each character in the string
    while(*string) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *string++);
    }
    glEnable(GL_LIGHTING);
}
// Routine to draw a stroke character string.
void writeStrokeString(void *font, const char *string)
{
    const char *c;
    for (c = string; *c != '\0'; c++) glutStrokeCharacter(font, *c);
}
// Helper function to draw triangles
void drawTriangles(float triangles[][4][3], int numTriangles) {
    for (int i = 0; i < numTriangles; ++i) {
        glBegin(GL_TRIANGLES);
        glNormal3fv(triangles[i][3]); // The normal for the triangle
        for (int j = 0; j < 3; ++j) { // Each triangle has 3 vertices
            glVertex3fv(triangles[i][j]); // Pass the vertex to OpenGL
        }
        glEnd();
    }
}
// Helper function to draw quads
void drawQuads(float quads[][5][3], int numQuads) {
    for (int i = 0; i < numQuads; ++i) {
        glBegin(GL_QUADS);
        glNormal3fv(quads[i][4]);
        for (int j = 0; j < 4; ++j) { // Each quad has 4 vertices
            glVertex3fv(quads[i][j]); // Pass the vertex to OpenGL
        }
        glEnd();
    }
}
void drawReflectiveQuads(float quads[][5][3], int numQuads) {
    for (int i = 0; i < numQuads; ++i) {
        // Extract each corner point
        float* v1 = quads[i][0];
        float* v2 = quads[i][1];
        float* v3 = quads[i][2];
        float* v4 = quads[i][3];
        float* normal = quads[i][4];

        // Calculate dimensions
        float width = sqrt(pow(v2[0] - v1[0], 2) + pow(v2[1] - v1[1], 2) + pow(v2[2] - v1[2], 2));
        float height = sqrt(pow(v4[0] - v1[0], 2) + pow(v4[1] - v1[1], 2) + pow(v4[2] - v1[2], 2));

        // Determine the number of large quads across the width and height (each segment is 10 units)
        int numWidth = ceil(width / 2);
        int numHeight = ceil(height / 2);

        // Vector direction for width and height (each segment is 10 units long)
        float widthVec[3] = {(v2[0] - v1[0]) / numWidth, (v2[1] - v1[1]) / numWidth, (v2[2] - v1[2]) / numWidth};
        float heightVec[3] = {(v4[0] - v1[0]) / numHeight, (v4[1] - v1[1]) / numHeight, (v4[2] - v1[2]) / numHeight};

        // Draw each large quad
        glBegin(GL_QUADS);
        for (int w = 0; w < numWidth; ++w) {
            for (int h = 0; h < numHeight; ++h) {
                // Calculate corners of the large quad
                float base[3] = {
                    v1[0] + w * widthVec[0] + h * heightVec[0],
                    v1[1] + w * widthVec[1] + h * heightVec[1],
                    v1[2] + w * widthVec[2] + h * heightVec[2]
                };

                float nextWidth[3] = {base[0] + widthVec[0], base[1] + widthVec[1], base[2] + widthVec[2]};
                float nextHeight[3] = {base[0] + heightVec[0], base[1] + heightVec[1], base[2] + heightVec[2]};
                float diagonal[3] = {nextWidth[0] + heightVec[0], nextWidth[1] + heightVec[1], nextWidth[2] + heightVec[2]};

                glNormal3fv(normal);
                glVertex3fv(base);
                glVertex3fv(nextWidth);
                glVertex3fv(diagonal);
                glVertex3fv(nextHeight);
            }
        }
        glEnd();
    }
}
// Function to draw a triangle fan circle in the XY plane
void drawCircleXY(float centerX, float centerY, float centerZ, float radius) {
    int numSegments = 360;  // Number of segments to approximate the circle
    float angle;

    glPushMatrix();  // Save the current transformation matrix
    glTranslatef(centerX, centerY, centerZ);  // Move to the circle's center position

    glBegin(GL_TRIANGLE_FAN);  // Start drawing the circle using triangle fan
    glVertex3f(0.0f, 0.0f, 0.0f);  // Center of the circle

    for (int i = 0; i <= numSegments; i++) {  // Loop through circle segments
        angle = 2.0f * M_PI * i / numSegments;  // Calculate angle
        glVertex3f(cos(angle) * radius, sin(angle) * radius, 0.0f);  // Calculate x, y from angle
    }

    glEnd();  // End drawing of circle
    glPopMatrix();  // Restore the previous transformation matrix
}
// Helper function to draw triangle strip circles, partial circles, washers in the XZ plane
void drawCircle(float cx, float cy, float cz, float innerRadius, float outerRadius, float startAngle, float endAngle) {
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glNormal3f(0, 1, 0);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 50; ++i) {
        float theta = startAngle + (endAngle - startAngle) * float(i) / float(50); // Current angle
        float cosTheta = cosf(theta);
        float sinTheta = sinf(theta);
        // Outer vertex
        float xOuter = outerRadius * cosTheta;
        float zOuter = outerRadius * sinTheta;
        glVertex3f(xOuter, 0.0f, zOuter); // Output vertex for outer radius
        // Inner vertex
        float xInner = innerRadius * cosTheta;
        float zInner = innerRadius * sinTheta;
        glVertex3f(xInner, 0.0f, zInner); // Output vertex for inner radius
    }
    glEnd();
    glPopMatrix();
}
void drawCircles(float circles[][7], int numCircles) {
    for (int i = 0; i < numCircles; ++i) {
        drawCircle(
            circles[i][0], circles[i][1], circles[i][2], // cx, cy, cz
            circles[i][3], circles[i][4], // innerRadius, outerRadius
            circles[i][5], circles[i][6]  // startAngle, endAngle
        );
    }
}
// Draw curved barrier wall
void drawCurvedWall(float cx, float cy, float cz, float innerRadius, float outerRadius, float startAngle, float endAngle) {
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glNormal3f(0, 1, 0);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 50; ++i) {
        float theta = startAngle + (endAngle - startAngle) * float(i) / float(50); // Current angle
        float cosTheta = cosf(theta);
        float sinTheta = sinf(theta);
        glNormal3f(-cosTheta, 0.0f, -sinTheta);
        // Outer vertex
        float xOuter = innerRadius * cosTheta;
        float zOuter = innerRadius * sinTheta;
        glVertex3f(xOuter, 0.0f, zOuter);
        // Inner vertex
        float xInner = innerRadius * cosTheta;
        float zInner = innerRadius * sinTheta;
        glVertex3f(xInner, -cy, zInner);
    }
    glEnd();
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 50; ++i) {
        float theta = startAngle + (endAngle - startAngle) * float(i) / float(50); // Current angle
        float cosTheta = cosf(theta);
        float sinTheta = sinf(theta);
        glNormal3f(cosTheta, 0.0f, sinTheta);
        // Outer vertex
        float xOuter = outerRadius * cosTheta;
        float zOuter = outerRadius * sinTheta;
        glVertex3f(xOuter, 0.0f, zOuter);
        // Inner vertex
        float xInner = outerRadius * cosTheta;
        float zInner = outerRadius * sinTheta;
        glVertex3f(xInner, -cy, zInner);
    }
    glEnd();
    glPopMatrix();
}
// Draw cylinder given 2 3D coordinates and a radius
void drawCylinder(float x1, float y1, float z1, float x2, float y2, float z2, float radius) {
    GLUquadric* quadric = gluNewQuadric();
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    float length = sqrt(dx * dx + dy * dy + dz * dz);
    glPushMatrix();
    glTranslatef(x1, y1, z1);
    // Align the cylinder to the line (x1, y1, z1) -> (x2, y2, z2)
    float angle = acos(dz/length) * 180.0 / M_PI; // Convert to degrees
    glRotatef(angle, -dy, dx, 0.0);
    gluCylinder(quadric, radius, radius, length, 20, 20);
    glPopMatrix();

    gluDeleteQuadric(quadric);
}
// Draw sliced cylinder. Used for shade covers on stop lights
void drawAngledSliceCylinder(float x, float y, float z) {
    GLUquadric* quadric = gluNewQuadric();
    int slices = 32;
    int stacks = 16;
    float radius = 1.5; // Radius of the cylinder
    float height = 2; // Height of the cylinder

    glPushMatrix();
    glTranslatef(x, y, z);
    glEnable(GL_CLIP_PLANE0);

    // Define the clipping plane
    // Normal vector at a 45-degree angle to the XY plane
    double plane[] = {0.0, 1.0, 1.0, 0.0};
    glClipPlane(GL_CLIP_PLANE0, plane);

    // Draw the cylinder
    gluCylinder(quadric, radius, radius, height, slices, stacks);

    glDisable(GL_CLIP_PLANE0);
    gluDeleteQuadric(quadric);
    glPopMatrix();
}
// Helper function to draw a box from corners
void drawBoxFromCorners(float x1, float y1, float z1, float x2, float y2, float z2) {
    float vertices[8][3] = {
        {std::min(x1, x2), std::min(y1, y2), std::min(z1, z2)},
        {std::max(x1, x2), std::min(y1, y2), std::min(z1, z2)},
        {std::max(x1, x2), std::max(y1, y2), std::min(z1, z2)},
        {std::min(x1, x2), std::max(y1, y2), std::min(z1, z2)},
        {std::min(x1, x2), std::max(y1, y2), std::max(z1, z2)},
        {std::max(x1, x2), std::max(y1, y2), std::max(z1, z2)},
        {std::max(x1, x2), std::min(y1, y2), std::max(z1, z2)},
        {std::min(x1, x2), std::min(y1, y2), std::max(z1, z2)}
    };
    int faces[6][4] = {{0, 1, 2, 3}, {7, 6, 5, 4}, {3, 2, 5, 4}, {0, 7, 6, 1}, {7, 4, 3, 0}, {1, 6, 5, 2}};
    float normals[6][3] = {{0, 0, -1}, {0, 0, 1}, {0, 1, 0}, {0, -1, 0}, {-1, 0, 0}, {1, 0, 0}};

    glBegin(GL_QUADS);
    for (int i = 0; i < 6; ++i) {
        glNormal3fv(normals[i]);
        for (int j = 0; j < 4; ++j) glVertex3fv(vertices[faces[i][j]]);
    }
    glEnd();
}


// Function to draw multiple boxes given an array of box corner coordinates
void drawMultipleBoxes(float boxes[][6], int numBoxes) {
    for (int i = 0; i < numBoxes; i++) {
        // Each 'boxes[i]' contains the parameters for 'drawBoxFromCorners'
        drawBoxFromCorners(boxes[i][0], boxes[i][1], boxes[i][2],
                           boxes[i][3], boxes[i][4], boxes[i][5]);
    }
}
void setOrthographicProjection() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void resetPerspectiveProjection() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
/*\ -------------------------- \*/


/*\ --- Drawing Functions ---- \*/
void drawGrass(void) {
    float extent = 10000.0f; // Large enough to cover the view
    glColor3f(1, 1, 1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureGrass[0]);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glNormal3f(0.0, 1.0, 0.0);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-extent, -0.10f, -extent);
        glTexCoord2f(0.0, 100.0); glVertex3f(-extent, -0.10f, extent);
        glTexCoord2f(100.0, 100.0); glVertex3f(extent, -0.10f, extent);
        glTexCoord2f(100.0, 0.0); glVertex3f(extent, -0.10f, -extent);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
void drawCloud(float x, float y, float z) {
    float cloudShade = day ? 0.9 : 0.2;
    glColor3f(cloudShade, cloudShade, cloudShade);
    glPushMatrix();
    glTranslatef(x, y, z);

    for (int i = 0; i < 8; i++) {
        glPushMatrix();
        glTranslatef(cloudParts[i].offsetX, cloudParts[i].offsetY, cloudParts[i].offsetZ);
        glutSolidSphere(cloudParts[i].size, 20, 20);
        glPopMatrix();
    }
    glPopMatrix();
}
void drawClouds() {
    glDisable(GL_LIGHTING);
    for (int i = 0; i < 6; i++) {
        glPushMatrix();
        glScalef(40, 40, 40);
        drawCloud(cloudPositions[i].x, cloudPositions[i].y, cloudPositions[i].z);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);
}
void drawHill(float x, float y, float z, float scale) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureGrass[0]);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, day ? GL_REPLACE : GL_MODULATE);

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale); // Adjust scale to desired hill size

    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &hillPoints[0][0][0]);
            glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 8, 2, &hillPoints[0][0][0]);  // Adjusted texture points
            glEnable(GL_MAP2_TEXTURE_COORD_2);
            glEnable(GL_MAP2_VERTEX_3);
            glMapGrid2f(10, 0.0, 1.0, 20, 0.0, 1.0);
            glEvalMesh2(GL_FILL, 0, 20, 0, 20);
        }
    }

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}
void drawTree(float x, float y, float z, float trunkHeight, float foliageRadius) {
    float trunkRadius = 3;  // Radius of the trunk
    float foliageHeight = 20.0f; // Height of the foliage

    glPushMatrix();

    // Position the entire tree
    glTranslatef(x, y, z);

    // Draw the trunk using custom drawCylinder
    glColor3f(0.55f, 0.27f, 0.07f); // Brown color for the trunk
    drawCylinder(0, 0, 0, 0, trunkHeight, 0, trunkRadius);

    // Draw the foliage as a cone using gluCylinder
    glColor3f(0.0f, 0.4f, 0.0f); // Green color for the foliage
    for(int i = 0; i < 3; i++){
        glPushMatrix();
        glTranslatef(0.0f, trunkHeight + (i * trunkHeight), 0.0f); // Move to the position for the foliage
        glRotatef(-90, 1, 0, 0);
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, foliageRadius - (i * 2.5), 0.0f, foliageHeight - (i * 2.5), 20, 20); // Cone: large base, zero at top
        gluDeleteQuadric(quad);
        glPopMatrix();
    }

    glPopMatrix();
}
void drawTrees() {
    for (const auto& tree : trees) {
        drawTree(tree.x, tree.y, tree.z, tree.trunkHeight, tree.treeHeight);
    }
    
    static std::vector<Tree> trees;
    static bool initialized = false;

    if (!initialized) {
        srand(time(nullptr)); // Seed random number generator
        // Generate random positions for trees
        for (int i = 0; i < 50; ++i) {  // Generate 50 trees
            float x, z;
            do {
                x = rand() % 1220 - 610;  // Range from -610 to 610
                z = rand() % 1640 - 820;  // Range from -820 to 820
            } while ((x > -290 && x < 330) && (z > -420 && z < 420));

            float trunkHeight = rand() % 4 + 6; // Random trunk height between 6 and 10
            float treeHeight = trunkHeight + (rand() % 5 + 5); // Total tree height will be trunk height plus 5 to 10

            trees.emplace_back(x, 0, z, trunkHeight, treeHeight);
        }
        initialized = true;
    }

    // Draw all trees
    for (const auto& tree : trees) {
        drawTree(tree.x, tree.y, tree.z, tree.trunkHeight, tree.treeHeight);
    }
}
void drawSun() {
    glPushMatrix();
    float sunRadius = 20.0f; // Large radius for the sun

    // Move to the position and draw the sun
    glTranslatef(400, 300, 1000); // Adjust these values based on your scene
    glColor3f(1.0f, 0.95f, 0.7f); // Sun color

    // Enable emissive material to make the sun glow
    GLfloat mat_emission[] = {0.9f, 0.8f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);

    // Create a quadric object to draw sphere
    GLUquadric* quadric = gluNewQuadric();
    gluSphere(quadric, sunRadius, 30, 30); // Draw sphere
    gluDeleteQuadric(quadric);

    // Reset emission material to none
    GLfloat no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

    glPopMatrix();
}
void drawMoon() {
    glPushMatrix();
    float moonRadius = 10.0f;

    // Move to the position and draw the sun
    glTranslatef(-400, 300, -1000); // Adjust these values based on your scene
    glColor3f(0.95f, 0.95f, 0.95f); // Sun color

    // Enable emissive material to make the sun glow
    GLfloat mat_emission[] = {0.9f, 0.9f, 0.9f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);

    // Create a quadric object to draw sphere
    GLUquadric* quadric = gluNewQuadric();
    gluSphere(quadric, moonRadius, 30, 30); // Draw sphere
    gluDeleteQuadric(quadric);

    // Reset emission material to none
    GLfloat no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

    glPopMatrix();
}
void drawStartFinishLine(void) {
    int numSegments = 20;  // Number of checkered segments
    float segmentLength = (280.0f - 200.0f) / numSegments;
    float startY = -5.0f;  // Starting y-coordinate for the checkered pattern
    float stripeHeight = 5.0f;  // Height of each stripe

    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    for (int j = 0; j < 2; j++) {  // Two rows of checkered patterns
        for (int i = 0; i < numSegments; ++i) {
            // Set color: alternate between white (1, 1, 1) and black (0, 0, 0)
            if ((i + j) % 2 == 0) {
                glColor3f(1.0f, 1.0f, 1.0f); // White
            } else {
                glColor3f(0.0f, 0.0f, 0.0f); // Black
            }
            
            // Calculate the left and right x-coordinates for the current segment
            float leftX = 280.0f - segmentLength * i;
            float rightX = leftX - segmentLength;
            
            // Draw one segment of the start/finish line
            glVertex3f(leftX, 0.5f, startY + j * stripeHeight);  // Top left
            glVertex3f(rightX, 0.5f, startY + j * stripeHeight); // Top right
            glVertex3f(rightX, 0.5f, startY + stripeHeight + j * stripeHeight);  // Bottom right
            glVertex3f(leftX, 0.5f, startY + stripeHeight + j * stripeHeight);   // Bottom left
        }
    }
    glEnd(); // End drawing
    glEnable(GL_LIGHTING);
}
void drawStartLight(){
    float baseX = 252.5f;
    float baseY = 32.0f;
    float baseZ = -3.1f;
    float yIncrement = 3.0f;
    float xDecrement = 5.0f;
    int numLightsPerRow = 6;
    float radius = 1.4f;
    float colors[4][3] = {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    
    // Main structure and base
    glColor3f(0.8f, 0.8f, 0.8f);
    drawCylinder(300, 40, 0, 180, 40, 0, 2);
    drawCylinder(290, 0, 0, 290, 40, 0, 2);
    drawCylinder(190, 0, 0, 190, 40, 0, 2);
    glColor3f(0, 0, 0);
    drawBoxFromCorners(225, 43, -2, 255, 30, -3);
    
    // Lights
    for (int row = 0; row < 4; row++) {
        for (int i = 0; i < numLightsPerRow; i++) {
            float currentX = baseX - i * xDecrement;
            float currentY = baseY + row * yIncrement;
            glColor3f(0.3, 0.3, 0.3);
            drawAngledSliceCylinder(currentX, currentY, -5.1); // Shade cover
            if (row <= currentLightRow) {
                GLfloat emissive[] = {colors[row][0], colors[row][1], colors[row][2], 1.0f};
                glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
            }
            glColor3fv(colors[row]); // Light color
            glNormal3f(0, 0, 1);
            drawCircleXY(currentX, currentY, baseZ, radius); // Actual light
            GLfloat nonEmissive[] = {0.0f, 0.0f, 0.0f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, nonEmissive);
        }
    }
}
void initConfetti(ConfettiParticle confetti[], float posX, float posY, float posZ) {
    for (int i = 0; i < MAX_CONFETTI; i++) {
        confetti[i].position[0] = posX;
        confetti[i].position[1] = posY;
        confetti[i].position[2] = posZ;
        confetti[i].velocity[0] = (rand() % 20 - 10) * 0.05;
        confetti[i].velocity[1] = (rand() % 30 + 10) * 0.25;
        confetti[i].velocity[2] = (rand() % 20 - 10) * 0.05;
        confetti[i].color[0] = rand() / (float)RAND_MAX;
        confetti[i].color[1] = rand() / (float)RAND_MAX;
        confetti[i].color[2] = rand() / (float)RAND_MAX;
        confetti[i].active = true;
    }
}
void updateAndDrawConfetti(ConfettiParticle confetti[]) {
    glPointSize(10.0); // Set point size for confetti particles
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_CONFETTI; i++) {
        if (confetti[i].active) {
            // Update position
            confetti[i].position[0] += confetti[i].velocity[0];
            confetti[i].position[1] += confetti[i].velocity[1];
            confetti[i].position[2] += confetti[i].velocity[2];

            // Apply gravity
            confetti[i].velocity[1] -= 0.1; // gravity effect

            // Draw particle
            glColor3fv(confetti[i].color);
            glVertex3fv(confetti[i].position);

            // Deactivate confetti that falls below a certain height
            if (confetti[i].position[1] < -1) {
                confetti[i].active = false;
            }
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}
void drawTrack(void){
    // Drawing the floor
    glColor3f(0.35, 0.35, 0.35);
    if(day){drawQuads(trackQuads, 9);}
    else{drawReflectiveQuads(trackQuads, 9);}
    drawCircles(trackCurves, 9);
    
    // Drawing the barriers
    glColor3f(0.75, 0, 0);
    drawMultipleBoxes(axisBarriers, axisBarriersCount);
    drawCircles(curveBarriers, curveBarriersCount);
    
    for (int i = 0; i < curveBarriersCount; ++i) {
        float* barrier = curveBarriers[i];
        drawCurvedWall(barrier[0], barrier[1], barrier[2], barrier[4], barrier[3], barrier[5], barrier[6]);
    }
}
void drawTeapot(void){
    glPushMatrix();
    glTranslatef(0.0f, 25.0f, 0.0f);
    glRotatef(teapotRotationAngle, 0, 1, 0);
    glColor3f(1.0f, 0.8f, 0.0f);
    glutSolidTeapot(40.0); // The parameter is the radius of the teapot
    glPopMatrix();
}
void updateHeadlights() {
    float rad = angleX * PI / 180.0;
    float lightDirX = sin(rad);
    float lightDirZ = cos(rad);
    float headlightOffsetX = 2; // distance to the left and right from the center
    float headlightHeight = 5; // height of the headlights from the ground
    float headlightForward = 18.0; // forward offset of the headlights from the center

    // Calculate positions of the left and right headlights
    GLfloat light0Pos[] = {
        meX - lightDirZ * headlightOffsetX + lightDirX * headlightForward,
        headlightHeight,
        meZ + lightDirX * headlightOffsetX + lightDirZ * headlightForward,
        1.0
    };
    GLfloat light1Pos[] = {
        meX + lightDirZ * headlightOffsetX + lightDirX * headlightForward,
        headlightHeight,
        meZ - lightDirX * headlightOffsetX + lightDirZ * headlightForward,
        1.0
    };

    // Define downward tilt of the headlights (negative y-component)
    GLfloat lightDir[] = {lightDirX, angleY, lightDirZ};

    // Set the light properties for the left headlight
    glLightfv(GL_LIGHT1, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDir);

    // Set the light properties for the right headlight
    glLightfv(GL_LIGHT2, GL_POSITION, light1Pos);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lightDir);
}

// Draw racecar wheel using torus
void drawWheel(float x, float y, float z, float angle) {
    float wheelWidth = 1.5f;
    float wheelRadius = 5.0f;
    glColor3f(0.0f, 0.0f, 0.0f);
    for(float i = 0.7; i <= 1; i += 0.05){
        glPushMatrix();
        glTranslatef(x * i, y, z + (abs(x)/x * 0.1 * (0.85 - i) * angle));
        glRotatef(90 + angle, 0.0f, 1.0f, 0.0f);
        glutSolidTorus(wheelWidth, wheelRadius, 30, 30);
        glPopMatrix();
    }
    glPushMatrix();
    glColor3f(0.75f, 0.75f, 0.75f);
    glTranslatef(x, y, z);
    glRotatef(-90 + angle, 0, 1, 0);
    drawCircleXY(0, 0, 0, 4);
    glPopMatrix();
}
void drawGaugeContent(void){
    int mph = static_cast<int>((velocity / 3.0) * 120);

    // Speed
    char mphText[10];
    sprintf(mphText, "%03d", abs(mph));
    glPushMatrix();
    glTranslatef(1.6, 11.5, 9.9);
    glScalef(0.015, 0.015, 0.015);
    glRotatef(180, 0.0, 1.0, 0.0);
    writeStrokeString(GLUT_STROKE_ROMAN, mphText);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.9, 13.5, 9.9);
    glScalef(0.005, 0.005, 0.005);
    glRotatef(180, 0.0, 1.0, 0.0);
    writeStrokeString(GLUT_STROKE_ROMAN, "SPEED");
    glPopMatrix();
    
    // Lap Time
    char timeText[10];
    sprintf(timeText, "%03d", int(currentLapTime));
    glPushMatrix();
    glTranslatef(-4, 11, 9.9);
    glScalef(0.01, 0.01, 0.01);
    glRotatef(180, 0.0, 1.0, 0.0);
    writeStrokeString(GLUT_STROKE_ROMAN, timeText);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(6, 11, 9.9);
    glScalef(0.01, 0.01, 0.01);
    glRotatef(180, 0.0, 1.0, 0.0);
    glutStrokeCharacter(GLUT_STROKE_ROMAN, (velocity >= 0) ? 'D' : 'R');
    glPopMatrix();
}
void drawRacecar(void){
    glPushMatrix();
    glTranslatef(meX, 0.0f, meZ);
    glRotatef(angleX, 0.0f, 1.0f, 0.0f);
    glScalef(0.4f, 0.4f, 0.4f);
    
    // Front and rear wings
    glColor3f(0.25, 0.25, 0.25);
    drawBoxFromCorners(-15, 0, 45, 15, 5, 35); // Front wing
    drawBoxFromCorners(-15, 15, -35, 15, 20, -45); // Rear wing
    
    // Body
    glColor3f(0.8, 0.0, 0.0);
    drawBoxFromCorners(-5, 0, 15, 5, 10, -35); // Central body
    drawBoxFromCorners(-15, 0, 5, 15, 10, -15); // Fenders
    drawBoxFromCorners(-5, 10, -5, 5, 15, -15); // Behind cockpit
    drawQuads(slantedQuads, 10);
    drawTriangles(slantedTriangles, 6);
    
    // Driver
    glColor3f(0.25, 0.25, 0.25);
    glPushMatrix();
    glTranslatef(0, 10, 0);
    glutSolidSphere(5, 100, 100);
    glPopMatrix();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Gauge cluster
    glColor4f(0, 0, 0, 0.5);
    drawCircleXY(0, 10, 10.1, 5);
    drawCircleXY(5, 10, 10.1, 3);
    drawCircleXY(-5, 10, 10.1, 3);
    
    // Steering wheel
    if(fpv){
        glColor4f(0, 0, 0, 1.0);
        drawCircleXY(0, 10, 7, 5);
        glColor3f(1, 0, 0);
        drawCircleXY(0, 10.1, 6.9, 4);
    }
    
    GLfloat mat_emission[] = {0.5f, 0.5f, 0.5f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
    glColor4f(0.1, 0.1, 0.1, 0.5);
    drawCircleXY(0, 10, 10.05, 4.9);
    drawCircleXY(5, 10, 10, 2.9);
    drawCircleXY(-5, 10, 10, 2.9);
    GLfloat no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
    glDisable(GL_BLEND);
    
    if(fpv){drawGaugeContent();}
    
    // Intakes
    glColor3f(0, 0, 0);
    float intakeTriangles[][4][3] = {
        {{-5, 10, -15}, {-5, 15, -15}, {-15, 10, -15}, {0, 0, 1}}, // left
        {{5, 10, -15}, {5, 15, -15}, {15, 10, -15}, {0, 0, 1}}, // right
    };
    drawTriangles(intakeTriangles, 2);
    
    // Wheels
    drawWheel(-12.5, 5, 20, wheelAngle);    // Front left wheel
    drawWheel(12.5, 5, 20, wheelAngle);     // Front right wheel
    drawWheel(-12.5, 5, -30, 0);   // Rear left wheel
    drawWheel(12.5, 5, -30, 0);    // Rear right wheel
    drawCylinder(-12.5, 5, 20, 12.5, 5, 20, 1);
    drawCylinder(-12.5, 5, -30, 12.5, 5, -30, 1);
    glPopMatrix();
    
    updateHeadlights();
}
void drawMPHDial(float mph) {
    float gaugeHeight = 20.0f; // Height of the gauge
    int baseX = 10; // Base x position
    int baseY = glutGet(GLUT_WINDOW_HEIGHT) - 30;
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(baseX, baseY);
    glVertex2f(baseX + abs(mph), baseY);
    glVertex2f(baseX + abs(mph), baseY + 20);
    glVertex2f(baseX, baseY + 20);
    glEnd();
    drawText((velocity >= 0) ? "DRIVE" : "REVERSE", 10, 965);
}

// Drawing routine.
void drawScene(void)
{
    if(day) {
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    } else {
        glClearColor(0.05, 0.05, 0.25, 1.0);
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float baseCameraX, baseCameraY, baseCameraZ;
    float targetX, targetY, targetZ;

    if (!gameStarted) {
        // Camera rotates around the origin at a radius of 200
        baseCameraX = 300 * cos(cameraAngle);
        baseCameraZ = 300 * sin(cameraAngle);
        baseCameraY = 100;  // Fixed height above the origin

        targetX = 0;  // Looking at the origin
        targetY = 0;
        targetZ = 0;

        cameraAngle += 0.005;  // Increment the angle for smooth rotation
        
        setOrthographicProjection();
        renderCenteredText("OpenGL Racing Simulator");
        resetPerspectiveProjection();
    } else {
        // Standard game camera logic
        float cameraDistance = fpv ? 0.1 : 50; // Distance behind the car
        float cameraHeight = fpv ? 10 : 50;   // Height above the car
        float sideOffset = 50.0f;     // Distance to the side of the car for side views

        baseCameraX = meX - cameraDistance * sin(angleX * PI / 180);
        baseCameraZ = meZ - cameraDistance * cos(angleX * PI / 180);
        baseCameraY = meY + cameraHeight;

        targetX = meX;  // Car's current position
        targetY = meY + 10;
        targetZ = meZ;

        if (lookBehind) {
            baseCameraX = meX + cameraDistance * sin(angleX * PI / 180);
            baseCameraZ = meZ + cameraDistance * cos(angleX * PI / 180);
        } else if (lookLeft) {
            baseCameraX = meX + sideOffset * cos(angleX * PI / 180);
            baseCameraZ = meZ - sideOffset * sin(angleX * PI / 180);
        } else if (lookRight) {
            baseCameraX = meX - sideOffset * cos(angleX * PI / 180);
            baseCameraZ = meZ + sideOffset * sin(angleX * PI / 180);
        }
    }

    // Setup the camera
    gluLookAt(baseCameraX, baseCameraY, baseCameraZ, // Camera position
              targetX, targetY, targetZ, // Look at point
              0.0f, 1.0f, 0.0f); // Up vector

    drawGrass();
    drawHill(-200, 0, 450, 90);
    drawClouds();
    drawTrees();
    drawTrack();
    drawStartFinishLine();
    drawStartLight();
    drawTeapot();
    drawRacecar();
    if(day){drawSun();}
    else{drawMoon();}
    
    if (timerRunning) {
        int currentTime = glutGet(GLUT_ELAPSED_TIME);
        currentLapTime = (currentTime - lapStartTime) / 1000.0f;
        char currentLapTimeText[100];
        sprintf(currentLapTimeText, "Current Lap Time: %.2f seconds", currentLapTime);
        setOrthographicProjection();
        drawText(currentLapTimeText, 10, 50);
        resetPerspectiveProjection();
    }

    if (currentCheckpoint > 6){
        updateAndDrawConfetti(confettiCannon1);
        updateAndDrawConfetti(confettiCannon2);
        
        char lapTimeText[100]; // Buffer for lap time text
        float lapTime = (lapEndTime - lapStartTime) / 1000.0f;
        sprintf(lapTimeText, "Lap completed in %.2f seconds.", lapTime);
        setOrthographicProjection();  // Switch to 2D projection
        drawText(lapTimeText, 10, 50);  // Draw text on the screen
        drawText("Press 'r' to restart.", 10, 70);  // Draw text on the screen
        resetPerspectiveProjection();  // Switch back to your 3D projection
    }
    if(!fpv){ // Third person view dials
        setOrthographicProjection();
        float mph = velocity * 25;
        drawMPHDial(mph); // Draw the MPH dial
        resetPerspectiveProjection();
    }

    glutSwapBuffers();
}
/*\ -------------------------- \*/


/*\ - Initialization Routine - \*/
void updateLightSequence(int value) {
    if (currentLightRow < 3) {
        currentLightRow++;
        glutPostRedisplay();  // Request a redraw to update the scene
        glutTimerFunc(lightUpdateTime, updateLightSequence, 0);  // Continue the timer
    }
}
void setup(void)
{
    glEnable(GL_DEPTH_TEST); // Enable depth testing.
    glEnable(GL_LIGHTING);
    
    // Sunlight
    float sunlightPos[] = {0.0, 100.0, 100.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, sunlightPos);

    // Common spotlight properties
    GLfloat lightDiffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lightSpecular[] = {1.0, 1.0, 1.0, 1.0};

    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);

    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecular);

    GLfloat spotCutoff[] = {60.0};
    glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, spotCutoff);
    glLightfv(GL_LIGHT2, GL_SPOT_CUTOFF, spotCutoff);
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb);  // Global ambient light.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // Enable local viewpoint.

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    currentLightRow = -1;
    updateLightSequence(0);
    
    initConfetti(confettiCannon1, 200.0, 0.0, 100);
    initConfetti(confettiCannon2, 280.0, 10.0, 100);
}
void startScreenResize(int w, int h) {
    // Avoid division by zero
    if (h == 0) h = 1;

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // Update the projection to match the new window size
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, w, 0.0, h);  // Map the complete window

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void resize(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(120,1,1,1000);
    glMatrixMode(GL_MODELVIEW);
}
void updateCheckpoint(float x, float z) {
    switch (currentCheckpoint) {
        case 0:
            if (z > 0 && !timerRunning) {
                currentCheckpoint++;
                lapStartTime = glutGet(GLUT_ELAPSED_TIME);
                timerRunning = true;
                std::cout << "Lap started!\n";
            }
            break;
        case 1:
            if (z > 280) currentCheckpoint++;
            break;
        case 2:
            if (x < -200) currentCheckpoint++;
            break;
        case 3:
            if (z < -320) currentCheckpoint++;
            break;
        case 4:
            if (x > 0 && z > 60) currentCheckpoint++;
            break;
        case 5:
            if (x > 240) currentCheckpoint++;
            break;
        case 6:
            if (z > 0) {
                currentCheckpoint++;
                timerRunning = false;
                lapEndTime = glutGet(GLUT_ELAPSED_TIME);
                float lapTime = (lapEndTime - lapStartTime) / 1000.0f;
                std::cout << "Lap completed in " << lapTime << " seconds.\n";
                currentLightRow = -1;
            }
            break;
    }
}

int isInsideAnyBox(float x, float z, float boxes[][6], int numBoxes) {
    float radius = 5.0f; // Radius of the circle around the point

    for (int i = 0; i < numBoxes; i++) {
        // Calculate expanded bounds to include the circle's radius
        float x1 = fmin(boxes[i][0], boxes[i][3]) - radius;
        float x2 = fmax(boxes[i][0], boxes[i][3]) + radius;
        float z1 = fmin(boxes[i][2], boxes[i][5]) - radius;
        float z2 = fmax(boxes[i][2], boxes[i][5]) + radius;

        // Check if the center of the circle is within the expanded bounds
        if (x >= x1 && x <= x2 && z >= z1 && z <= z2) {
            return 1; // Center of the circle is within an expanded box
        }
    }
    return 0; // Center of the circle is not inside any expanded box
}
bool isWithinAngles(float x, float z, float cx, float cz, float startAngle, float endAngle) {
    float angleToPoint = atan2(z - cz, x - cx) * 180.0 / PI;
    if (angleToPoint < 0) angleToPoint += 360;

    float normalizedStartAngle = fmod(startAngle, 360);
    float normalizedEndAngle = fmod(endAngle, 360);

    if (normalizedStartAngle <= normalizedEndAngle) {
        return angleToPoint >= normalizedStartAngle && angleToPoint <= normalizedEndAngle;
    } else {
        return angleToPoint >= normalizedStartAngle || angleToPoint <= normalizedEndAngle;
    }
}
// Function to determine if a point (x, z) intersects with any annular or partial ring barrier.
int isInsideAnyCircle(float x, float z, float circles[][7], int numCircles) {
    float carRadius = 6;
    for (int i = 0; i < numCircles; i++) {
        // Circle center and radii
        float cx = circles[i][0];
        float cz = circles[i][2];
        float outerRadius = circles[i][3] + carRadius;
        float innerRadius = circles[i][4] - carRadius;
        float startAngle = circles[i][5] * 180.0 / PI;
        float endAngle = circles[i][6] * 180.0 / PI;
        float outerRadiusSquared = outerRadius * outerRadius;
        float innerRadiusSquared = innerRadius * innerRadius;

        // Calculate squared distance from point to circle center
        float dx = x - cx;
        float dz = z - cz;
        float distSquared = dx * dx + dz * dz;

        // Check if within annular range
        if (distSquared <= outerRadiusSquared && distSquared >= innerRadiusSquared && isWithinAngles(x, z, cx, cz, startAngle, endAngle)) {
            return 1; // Collision detected
        }
    }
    return 0; // No collision detected
}
void update(int value) {
    if(day){
        glEnable(GL_LIGHT0);  // Sunlight
        if (headlightMode == 3) {
            glDisable(GL_LIGHT1); // Disable left headlight
            glDisable(GL_LIGHT2); // Disable right headlight
        }
    } else {
        glDisable(GL_LIGHT0); // Disable sunlight
        if (headlightMode == 3) {
            glEnable(GL_LIGHT1);  // Enable left headlight
            glEnable(GL_LIGHT2);  // Enable right headlight
        }
    }
    
    if (keyStates['w']) { // Accelerate
        velocity += acceleration;
        if (velocity > maxVelocity) velocity = maxVelocity;
    } else if (keyStates['s'] && lapStartTime) { // Decelerate
        velocity -= deceleration;
        if (velocity < -maxVelocity) velocity = -maxVelocity;
    } else { // Automatic deceleration when no keys are pressed
        if (velocity > 0) velocity -= deceleration;
        else if (velocity < 0) velocity += deceleration;
        if (std::abs(velocity) < deceleration) velocity = 0; // Stop completely if speed is very low
    }

    // Define the maximum wheel angle
    const float maxWheelAngle = 25.0f;
    const float wheelAngleStep = 5.0f;  // Adjust this to control the smoothness

    // Handling turning while moving
    if (velocity != 0 && lapStartTime) {
        float turnAdjustment = (fabs(velocity) <= 2) ?
            (turnSpeed * 0.5 * (velocity > 0 ? 1 : -1)) :
            (turnSpeed * (1.0 - 0.5 * (fabs(velocity) / maxVelocity)) * (velocity > 0 ? 1 : -1));

        if (keyStates['a']) { // Turn left
            angleX += turnAdjustment;
            wheelAngle += wheelAngle < maxWheelAngle ? wheelAngleStep : 0;
            wheelAngle = std::min(wheelAngle, maxWheelAngle); // Ensure it does not exceed max angle
        } else if (keyStates['d']) { // Turn right
            angleX -= turnAdjustment;
            wheelAngle -= wheelAngle > -maxWheelAngle ? wheelAngleStep : 0;
            wheelAngle = std::max(wheelAngle, -maxWheelAngle); // Ensure it does not exceed min angle
        }
    } else {
        // If neither 'a' nor 'd' is pressed or the car is not moving, gradually return the wheel to the center
        if (wheelAngle < 0) {
            wheelAngle += wheelAngleStep;
            wheelAngle = std::min(wheelAngle, 0.0f); // Do not overshoot the center
        } else if (wheelAngle > 0) {
            wheelAngle -= wheelAngleStep;
            wheelAngle = std::max(wheelAngle, 0.0f); // Do not overshoot the center
        }
    }

    // Check if the proposed new position is within any barriers and then update position
    float proposedMeZ = meZ + velocity * cos(angleX * PI / 180);
    float proposedMeX = meX + velocity * sin(angleX * PI / 180);
    if (!isInsideAnyBox(proposedMeX, proposedMeZ, axisBarriers, axisBarriersCount) &&
        !isInsideAnyCircle(proposedMeX, proposedMeZ, curveBarriers, curveBarriersCount)) {
        // If not inside any box, update the position
        meZ = proposedMeZ;
        meX = proposedMeX;
    } else {
        // Collision detected, apply bounce back
        float elasticity = 0.25; // Coefficient of how much velocity is preserved after the bounce
        velocity = -velocity * elasticity; // Reverse and reduce velocity

        // Recalculate position using adjusted velocity
        meZ += velocity * cos(angleX * PI / 180);
        meX += velocity * sin(angleX * PI / 180);
    }
    
    if(!useIdleFunc){
        for (int i = 0; i < 6; i++) {
            // Update cloud position
            cloudPositions[i].z += cloudSpeed;

            // Reset cloud position if it moves too far
            if (cloudPositions[i].z > 25.0) {
                cloudPositions[i].z = -25.0;
            }
        }
        teapotRotationAngle += 2.0f; // Increase the angle by 2 degrees each frame
        if (teapotRotationAngle > 360.0f) {
            teapotRotationAngle -= 360.0f; // Wrap around at 360 degrees
        }
    }
    glutPostRedisplay(); // Redraw the scene
    glutTimerFunc(16, update, 0); // Re-register timer for continuous updates
    
    updateCheckpoint(meX, meZ);
}
void keyInput(unsigned char key, int x, int y) {
    key = tolower(key);
    keyStates[key] = true;
    switch (key) {
        case 'c':
            lookBehind = true;
            break;
        case 'q':
            lookLeft = true;
            break;
        case 'e':
            lookRight = true;
            break;
        case 'x':
            fpv = !fpv;
            break;
        case 'h':
            headlightMode = (headlightMode + 1) % 4;  // Cycle through headlights
            if(headlightMode){
                glEnable(GL_LIGHT1);
                glEnable(GL_LIGHT2);
            } else {
                glDisable(GL_LIGHT1);
                glDisable(GL_LIGHT2);
            }
            angleY = (headlightMode == 1 ? -1.25 : -1);
            break;
        case 'r':
            meX=240, meY=0, meZ=-40, angleX=0, angleY = (headlightMode == 2 ? -1 : -1.25);
            currentCheckpoint = 0;
            timerRunning = false;
            lapStartTime = 0;
            velocity = 0;
            currentLightRow = -1;
            updateLightSequence(0);
            break;
        case 27: // ASCII code for the ESC key
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void keyUp(unsigned char key, int x, int y) {
    key = tolower(key);
    keyStates[key] = false;
    if (key == 'a' || key == 'd') {
        wheelAngle = 0.0f;
    }
    if (key == 'c') {
        lookBehind = false;
    } else if (key == 'q') {
        lookLeft = false;
    } else if (key == 'e') {
        lookRight = false;
    }
}
void specialKeyInput(int key, int x, int y)
{
    switch(key){
        // For clipping through walls
        case GLUT_KEY_UP:
            meZ=meZ+stepsize*cos(angleX*PI/180);
            meX=meX+stepsize*sin(angleX*PI/180);
            break;
        case GLUT_KEY_DOWN:
            meZ=meZ-stepsize*cos(angleX*PI/180);
            meX=meX-stepsize*sin(angleX*PI/180);
            break;
        case GLUT_KEY_RIGHT:
            angleX-=turnsize;
            break;
        case GLUT_KEY_LEFT:
            angleX+=turnsize;
            
            break;
    }
    glutPostRedisplay();
}
void menu(int item) {
    switch(item) {
        case 1: // Toggle Day/Night
            day = !day; // Toggle the day variable
            
            if(day){glClearColor(0.53f, 0.81f, 0.92f, 1.0f);}
            else{glClearColor(0.05, 0.05, 0.15, 1.0);}
            break;
        case 2:
            break;
    }
    glutPostRedisplay(); // Redraw the scene to reflect the changes
}
void createMenu() {
    // Create a menu
    int menuId = glutCreateMenu(menu);

    // Add entries to the menu
    glutAddMenuEntry("Toggle Day/Night", 1);
    glutAddMenuEntry("Other Option", 2);

    // Attach the menu to the right mouse button
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}
void globalTimer(int value) {
    if(!gameStarted){
        // Post redisplay for the start window
        glutSetWindow(startWindow);
        glutPostRedisplay();
        
        // Post redisplay for the main game window
        glutSetWindow(mainWindow);
        glutPostRedisplay();
        
        // Re-register the timer callback to keep the updates coming
        glutTimerFunc(16, globalTimer, 0); // Approximately 60 times per second
    }
}
void idle() {
    for (int i = 0; i < 6; i++) {
        // Update cloud position
        cloudPositions[i].z += cloudSpeed;

        // Reset cloud position if it moves too far
        if (cloudPositions[i].z > 25.0) {
            cloudPositions[i].z = -25.0;
        }
    }
    teapotRotationAngle += 2.0f; // Increase the angle by 2 degrees each frame
    if (teapotRotationAngle > 360.0f) {
        teapotRotationAngle -= 360.0f; // Wrap around at 360 degrees
    }
    glutPostRedisplay();
}
// Routine to output interaction instructions to the C++ window.
void printInteraction(void) {
    cout << "Interaction Instructions:" << endl;
    cout << "General Controls:" << endl;
    cout << "\tESC - Exit the game." << endl;
    cout << "\tR - Reset the game state, repositioning the vehicle and resetting lap timing and velocity." << endl;
    
    cout << "Movement Controls:" << endl;
    cout << "\tW - Accelerate the vehicle. Speed increases until it reaches a maximum limit." << endl;
    cout << "\tS - Decelerate or reverse the vehicle. Speed decreases, and the vehicle can move backward." << endl;
    cout << "\tA - Steer left. Adjusts the wheel angle to the left, up to a maximum angle." << endl;
    cout << "\tD - Steer right. Adjusts the wheel angle to the right, down to a minimum angle." << endl;

    cout << "Camera and View Controls:" << endl;
    cout << "\tC - Toggle rear view (look behind)." << endl;
    cout << "\tQ - Look left without changing the direction of movement." << endl;
    cout << "\tE - Look right without changing the direction of movement." << endl;
    cout << "\tX - Toggle between first-person view (FPV) and third-person view." << endl;

    cout << "Miscellaneous Controls:" << endl;
    cout << "\tH - Cycle through different headlight modes: Auto, Off, Low, High." << endl;

    cout << "Stepping Controls (stepping through walls):" << endl;
    cout << "\tArrow Up - Move forward relative to the vehicle’s current direction." << endl;
    cout << "\tArrow Down - Move backward relative to the vehicle’s current direction." << endl;

    cout << "Steering Controls:" << endl;
    cout << "\tArrow Left - Rotate the vehicle’s view left without moving." << endl;
    cout << "\tArrow Right - Rotate the vehicle’s view right without moving." << endl;

    cout << "Mouse Controls:" << endl;
    cout << "\tLeft click start window to select options and start the game." << endl;
    cout << "\tRight click gameplay window to use popup menu and change day/night settings." << endl;
}

void drawButtonLabel(const char* text, int x, int y, int width) {
    glColor3f(1.0, 1.0, 1.0);  // Text color
    glRasterPos2i(x + (width - glutBitmapLength(GLUT_BITMAP_8_BY_13, (const unsigned char*)text)) / 2, y - 7);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
}
void drawStartScreen() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 500, 0.0, 1000);  // Orthographic projection for 2D rendering

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);  // Set the background color

    glColor3f(1.0, 1.0, 1.0);  // Set text color to white
    glRasterPos2i(175, 800);  // Position the main title text
    const char* title = "Select Mode";
    for (const char* c = title; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    int startX = 150;
    int buttonWidth = 200;
    int buttonHeight = 50;

    // Day Button
    glColor3f(isDaySelected ? 0.0 : 0.3, isDaySelected ? 1.0 : 0.6, 0.0);  // Change color when selected
    glRecti(startX, 600, startX + buttonWidth, 650);
    drawButtonLabel("Day", startX, 625, buttonWidth);

    // Night Button
    glColor3f(isNightSelected * 0.2, isNightSelected * 0.2, isNightSelected ? 1.0 : 0.6);  // Change color when selected
    glRecti(startX, 500, startX + buttonWidth, 550);
    drawButtonLabel("Night", startX, 525, buttonWidth);

    // Play Game Button
    glColor3f(1.0, 0.0, 0.0);
    glRecti(startX, 400, startX + buttonWidth, 450);
    drawButtonLabel("Start Game", startX, 425, buttonWidth);

    glutSwapBuffers();
}
void switchToMainGame() {
    
    glutSetWindow(startWindow);
    glutHideWindow();

    glutSetWindow(mainWindow);
    glutShowWindow();
    
    meX=240, meY=0, meZ=-40, angleX=0, angleY = (headlightMode == 2 ? -1 : -1.25);
    currentCheckpoint = 0;
    timerRunning = false;
    lapStartTime = 0;
    velocity = 0;
    currentLightRow = -1;
    updateLightSequence(0);
    gameStarted = true;
    
    glutPostRedisplay();
}
void mouseInput(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
        int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

        // Transform y coordinate
        y = windowHeight - y;  // Adjust for OpenGL's coordinate system (bottom-up)

        // Button dimensions and positions might also need to be scaled
        int startX = (int)(150 * (windowWidth / 500.0));  // Scale position based on window width
        int buttonWidth = (int)(200 * (windowWidth / 500.0));  // Scale width
        int buttonHeight = 50;  // Fixed height (can also scale if necessary)

        // Check button boundaries
        if (x > startX && x < startX + buttonWidth) {
            if (y > 600 * (windowHeight / 1000.0) && y < (600 * (windowHeight / 1000.0) + buttonHeight)) {
                isDaySelected = true;
                isNightSelected = false;
                day = true;
            } else if (y > 500 * (windowHeight / 1000.0) && y < (500 * (windowHeight / 1000.0) + buttonHeight)) {
                isDaySelected = false;
                isNightSelected = true;
                day = false;
            } else if (y > 400 * (windowHeight / 1000.0) && y < (400 * (windowHeight / 1000.0) + buttonHeight)) {
                switchToMainGame();
            }
            glutPostRedisplay();
        }
    }
}

void createStartWindow() {
    glutInitWindowSize(500, 1000);
    glutInitWindowPosition(1000, 0);
    startWindow = glutCreateWindow("Start Screen");

    glutDisplayFunc(drawStartScreen);
    glutReshapeFunc(startScreenResize);
    glutMouseFunc(mouseInput);
}
void createMainWindow() {
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(0, 0);
    mainWindow = glutCreateWindow("OpenGL Racing Simulator");

    setup();  // Setup your OpenGL context and initial states for the main game
    createMenu();
    loadGrassTexture();

    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialKeyInput);
    glutTimerFunc(0, update, 0);
    glutTimerFunc(0, globalTimer, 0);

    if (useIdleFunc) {
        glutIdleFunc(idle);
    }
}
/*\ -------------------------- \*/
// Main routine.
int main(int argc, char **argv)
{
    printInteraction();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    createStartWindow();
    createMainWindow();

    glutMainLoop();
    return 0;
}
