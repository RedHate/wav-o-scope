/*
 * RedHate's 3d scope project feb 2021
 * Using Wizardyesterday's DspBuildingBlocks! :D
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include "DspBuildingBlocks/include/Nco.h"

#define WINDOW_TITLE    "Ultros - wav-o-scope."
#define PI              3.141593f
#define MAX (32768)
#define RATE (24000)

typedef struct uv{
    float   u,
            v;
}uv;

typedef struct rgba{
    float   r,
            g,
            b,
            a;
}rgba;

typedef struct vertex{
    float   x,
            y,
            z;
}vertex;

typedef struct vertex_tcnv{
    uv          t;
    rgba        c;
    vertex      n,
                v;
} vertex_tcnv;

static bool running 	= true;
vertex_tcnv obj[MAX];

bool IsKeyDown( SDL_Keycode k ){
    int numKeys;
    static const Uint8* kb_keys = NULL;
    kb_keys = SDL_GetKeyboardState( &numKeys );
    return kb_keys[SDL_GetScancodeFromKey( k )] ? true : false;
}

static void Perspective(float yfov, float aspect, float znear, float zfar){
    float
    ymax = znear * tan( yfov * PI / 360.0f ),
    ymin = -ymax,
    xmin = ymin * aspect,
    xmax = ymax * aspect;
    glFrustum(xmin,xmax,ymin,ymax,znear,zfar); //glFrustumf
}

void *update(void *argv){

    printf("Update thread initializing\r\n");
    
	int16_t sample;
	float floatSample;
	float iValue, qValue;
	Nco *ncoPtr = new Nco(RATE,atof("980.0f"));
	int r=1,g=0,b=1;
    
    while(running){
		
		int i;
		for (i=0;i<MAX;i++){

			//tone A mutation
			ncoPtr->run(&iValue,&qValue);
			floatSample = iValue;
			
			//count
			floatSample *= MAX-1;
			floatSample /= 1;
			
			if(r) r=0; else r=1;
			if(g) g=0; else g=1;
			if(b) b=0; else b=1;
			
			obj[i] = { 1, 1, {r,g,b,1}, 0,  0,  1,   i*10,   floatSample/500.0f,   0 };

			//cast it to the correct type..
			sample = (int16_t)floatSample;
			
			//write it to stdout to be picked up with: scope | aplay -f s16_le -r 24000
			//fwrite(&sample,sizeof(int16_t),1,stdout); //absolutely fucking gags the program lol

		}
    }

	delete ncoPtr;

    return NULL;
}

int main(int argc, char** argv){

	//some for the window
	int WINDOW_WIDTH  		= 854;
	int WINDOW_HEIGHT 		= 128;
    int fullscreen = 0;
	 
    //init sdl
    SDL_Init( SDL_INIT_VIDEO );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

    //create SDL window
    static SDL_Window* a_window = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
                             /*SDL_WINDOW_INPUT_GRABBED |*/ SDL_WINDOW_OPENGL );

    //create a GL context with SDL
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(a_window);
    SDL_GL_MakeCurrent(a_window, gl_ctx);

    //culling depth and hinting
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    //set viewport geometry
    glDepthRange(0.0f, 1.0f);
    glPolygonOffset( WINDOW_WIDTH, WINDOW_HEIGHT);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glScissor(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	float zoom = 100.0f;

    pthread_t update_thread;
    if(pthread_create(&update_thread, NULL, &update, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    while(running){

        //poll the mouse
        SDL_Event ev;
        while(SDL_PollEvent(&ev)){
            if(ev.type == SDL_QUIT){ running = false; }
            if(ev.type == SDL_WINDOWEVENT){
                if ( ev.window.event == SDL_WINDOWEVENT_RESIZED ){
                    int dw, dh;
                    WINDOW_WIDTH  = ev.window.data1;
                    WINDOW_HEIGHT = ev.window.data2;
                    SDL_GL_GetDrawableSize(a_window, &dw, &dh);
                    glPolygonOffset(dw, dh);
                    glViewport(0, 0, dw, dh);
                    glScissor(0, 0, dw, dh);
                }
            }
            if(ev.type == SDL_KEYDOWN){
                if ( ev.key.repeat ) break;
                SDL_Keycode wParam = ev.key.keysym.sym;
                //init shut down sequence implode destroy destroy!
                if(wParam == SDLK_ESCAPE){ running = false; }
                if(wParam == SDLK_z){ if(zoom < 200.0f) zoom = 500.0f; else zoom -= 100.0f; }
            }
        }

        //set up scene
        glClearColor(0.01f, 0.01f, 0.01f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//viewport geometry and camera
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); {
			Perspective(75.0f, (float)WINDOW_WIDTH/WINDOW_HEIGHT, 0.1f, 1000.0f);
			glRotatef(0, 1, 0, 0);
			glRotatef(0, 0, 1, 0);
			glRotatef(0, 0, 0, 1);
			glTranslatef(0, 0, -zoom);
		}

		//model projection
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		{
			glTranslatef(-MAX/2, 0, 0); //max z depth
			glRotatef(0, 1, 0, 0);
			glRotatef(0, 0, 1, 0);
			glRotatef(0, 0, 0, 1);
		}

		glEnable(GL_COLOR_MATERIAL);
		glPushMatrix();

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 12*sizeof(float), &obj->t);
		glColorPointer(4, GL_FLOAT,  12*sizeof(float), &obj->c);
		glNormalPointer(GL_FLOAT,  12*sizeof(float), &obj->n);
		glVertexPointer(3, GL_FLOAT, 12*sizeof(float), &obj->v);
		glDrawArrays(GL_LINE_STRIP, 0, MAX);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

        //flipity flip
        SDL_GL_SwapWindow(a_window);

    }

    if(pthread_join(update_thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    //shut down SDL
    SDL_DestroyWindow(a_window);

    return 0;

}

