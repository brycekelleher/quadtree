#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

#define WINDOW_WIDTH	640
#define WINDOW_HEIGHT	640

static int windowwidth		= WINDOW_WIDTH;
static int windowheight		= WINDOW_HEIGHT;

typedef struct qtnode_s
{
	struct qtnode_s* parent;
	struct qtnode_s* child[4];

	vec2_t		min, max;
	bool		solid;

} qtnode_t;

// mouse control
static bool lmbutton = false;
static bool rmbutton = false;
static float worldx, worldy;

// tools
static int 	rendermode = 1;
static float	toolsize = 4.0f;

// quad tree nodes
static int		numnodes;
static qtnode_t		nodes[4194304];

static qtnode_t *AllocNode()
{
	qtnode_t *n = nodes + numnodes;
	numnodes++;

	return n;
}

static bool BoxesIntersect(vec2_t mina, vec2_t maxa, vec2_t minb, vec2_t maxb)
{
	if(maxa[0] < minb[0] || mina[0] > maxb[0])
		return false;
	if(maxa[1] < minb[1] || mina[1] > maxb[1]) 
		return false;
	
	return true;
}

static void BuildQuadTree_r(qtnode_t* node, vec2_t min, vec2_t max, int level, int numlevels)
{
	vec2_t half;
	int i;

	// setup the node
	node->min	= min;
	node->max	= max;
	node->solid	= true;

	// don't recurse if we're at the bottom of the tree
	if(level == numlevels - 1)
		return;

	half[0] = 0.5f * (max[0] - min[0]);
	half[1] = 0.5f * (max[1] - min[1]);

	for(i = 0; i < 4; i++)
	{
		// allocate a node
		node->child[i] = AllocNode();
		node->child[i]->parent = node;

		if(i == 0)
		{
			min[0] = node->min[0];
			min[1] = node->min[1];
		}
		else if(i == 1)
		{
			min[0] = node->min[0] + half[0];
			min[1] = node->min[1];
		}
		else if(i == 2)
		{
			min[0] = node->min[0];
			min[1] = node->min[1] + half[1];
		}
		else if(i == 3)
		{
			min[0] = node->min[0] + half[0];
			min[1] = node->min[1] + half[1];
		}

		max = min + half;	

		// recurse this node
		BuildQuadTree_r(node->child[i], min, max, level + 1, numlevels);
	}
}

static void BuildQuadTree(int numlevels)
{
	vec2_t min, max;
	
	numnodes	= 1;
	min		= vec2_t(0, 0);
	max		= vec2_t(256.0f, 256.0f);

	BuildQuadTree_r(nodes, min, max, 0, numlevels);
}

static void RebuildSolids_r(qtnode_t* node)
{
	int i;

	if (!node)
		return;
	
	// need to rebuild the entire tree?
	//if (!BoxesIntersect(node->min, node->max, min, max))
	//	return 

	for (i = 0; i < 4; i++)
		RebuildSolids_r(node->child[i]);

	// update solidness if we're not a leaf node
	// (even more of a reason to do a bottom up refresh)
	if(node->child[0] && node->child[1] && node->child[2] && node->child[3])
	{
		node->solid = node->child[0]->solid && node->child[1]->solid && node->child[2]->solid && node->child[3]->solid;
	}
}

static void RebuildSolids()
{
	RebuildSolids_r(nodes);
}

static void CarveTree_r(qtnode_t* node, vec2_t min, vec2_t max)
{
	int i;

	if(!node)
		return;
	if(!BoxesIntersect(node->min, node->max, min, max))
		return;

	// recurse the children
	for (i = 0; i < 4; i++)
		CarveTree_r(node->child[i], min, max);

	// is this a leaf node
	if(!node->child[0] && !node->child[1] && !node->child[2] && !node->child[3])
	{
		// carve out a circle
		vec2_t mid0 = 0.5f * (max + min);
		vec2_t mid1 = 0.5f * (node->max + node->min);

		if((mid1 - mid0).Length() < toolsize)
		{
			if(lmbutton && !rmbutton)
				node->solid = false;
			if(!lmbutton && rmbutton)
				node->solid = true;
		}

		return;
	}
}

static void CarveTree(vec2_t min, vec2_t max)
{
	CarveTree_r(nodes, min, max);
}

// this rebuilds from the top down. it should be possible to rebuild from the bottom up and early out
// on the first node that hasn't changed?
#include "GL/freeglut.h"

static void R_DrawCircle(vec2_t pos, float r)
{
	float da = 6.2831f / 32.0f;

	glBegin(GL_LINE_LOOP);
	{
		for(int i = 0; i < 32; i++)
		{
			glVertex2f(
				r * cos(i * da) + pos[0],
				r * sin(i * da) + pos[1]);
		}
	}
	glEnd();
}

static void R_DrawCursor()
{
	glColor3f(0.5f, 0.5f, 1.0f);
	R_DrawCircle(vec2_t(worldx, worldy), toolsize);
}

static void R_DrawSolidQuad(vec2_t min, vec2_t max)
{
	vec2_t delta = max - min;

	glColor3f(1, 0, 0);
	glBegin(GL_QUADS);
	{
		glVertex2f(min[0], min[1]);
		glVertex2f(min[0] + delta[0], min[1]);
		glVertex2f(min[0] + delta[0], min[1] + delta[1]);
		glVertex2f(min[0], min[1] + delta[1]);
	}
	glEnd();
}

static void R_DrawWireframeQuad(vec2_t min, vec2_t max)
{
	vec2_t delta = max - min;

	glColor3f(1, 0, 0);

	glBegin(GL_LINE_LOOP);
	{
		glVertex2f(min[0], min[1]);
		glVertex2f(min[0] + delta[0], min[1]);
		glVertex2f(min[0] + delta[0], min[1] + delta[1]);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	{
		glVertex2f(min[0], min[1]);
		glVertex2f(min[0] + delta[0], min[1] + delta[1]);
		glVertex2f(min[0], min[1] + delta[1]);
	}
	glEnd();
}

static void R_DrawTree_r(qtnode_t* node)
{
	if(!node)
		return;

	static void (*QuadDrawList[])(vec2_t, vec2_t) =
	{
		R_DrawSolidQuad,
		R_DrawWireframeQuad,
	};

	if(node->solid)
	{
		QuadDrawList[rendermode](node->min, node->max);

		return;
	}

	for(int i = 0; i < 4; i++)
		R_DrawTree_r(node->child[i]);
}

static void glutDisplay()
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);

	R_DrawTree_r(nodes);

	R_DrawCursor();

	glutSwapBuffers();
}

static void glutReshape(int w, int h)
{
	windowwidth = w;
	windowheight = h;

	glViewport(0, 0, windowwidth, windowheight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 256, 0, 256, -1, 1);
}

static void glutKeyboard(unsigned char c, int x, int y)
{
	if(c == 'c')
	{
		void* buffer;
		FILE* fp;

		buffer = malloc(windowwidth * windowheight);
		glReadPixels(0, 0, windowwidth, windowheight, GL_BLUE, GL_UNSIGNED_BYTE, (void*)buffer);
		fp = fopen("dump.raw", "wb");
		fwrite(buffer, windowwidth * windowheight, 1, fp);
		fclose(fp);
		free(buffer);
	}

	if(c == 'r')
	{
		rendermode++;
		if(rendermode == 2)
			rendermode = 0;
	}

	if(c == 'x')
		toolsize *= 2.0f;
	if(c == 'z')
		toolsize /= 2.0f;

	glutPostRedisplay();
}

static void glutSpecial(int key, int x, int y)
{}

static void glutMouse(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		lmbutton = true;
	if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		lmbutton = false;

	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		rmbutton = true;
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		rmbutton = false;
}

static void UpdateMouse(int x, int y)
{
	float xx, yy;

	xx = (float)x / (float)(windowwidth);
	yy = (float)y / (float)(windowheight);
	yy = 1.0f - yy;

	xx *= 256.0f;
	yy *= 256.0f;

	// update the global cursor position state
	worldx = xx;
	worldy = yy;
}

static void glutMouseMotion(int x, int y)
{
	UpdateMouse(x, y);

	if(lmbutton || rmbutton)
	{
		vec2_t min, max;
		min = vec2_t(worldx, worldy) - vec2_t(toolsize, toolsize);
		max = vec2_t(worldx, worldy) + vec2_t(toolsize, toolsize);

		CarveTree(min, max);
		RebuildSolids();
	}

	glutPostRedisplay();
}

static void glutPassiveMouseMotion(int x, int y)
{
	UpdateMouse(x, y);

	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	// Glut Init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow(argv[0]);

	glutDisplayFunc(glutDisplay);
	glutReshapeFunc(glutReshape);
	glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecial);
	glutMouseFunc(glutMouse);
	glutMotionFunc(glutMouseMotion);
	glutPassiveMotionFunc(glutPassiveMouseMotion);

	BuildQuadTree(9);
	printf("done!\n");

	glutPostRedisplay();
	glutMainLoop();

	return 0;
}