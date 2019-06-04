#include <cstdio>
#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"

struct point
{
	float x;
	float y;
};

struct line
{
	vector<point> points;
	float width;
	float r,g,b,a;
};
struct layer
{
	vector<line> lines;
};
struct frame
{
	vector<layer> layers;
	size_t curLayer;
	int alpha;
};
struct toon
{
	vector<frame> frames;
	size_t framerate;
};

struct renderedFrame
{
	GLuint frameTex, miniFrameTex;
	vector<GLuint> layerTex, miniLayerTex;
};

SDL_Window* window;
int window_w,window_h;
toon myToon;
bool needRedraw=true;
int playingFrame;

int menuHeight, menuWidth;

//vector<int> curLayers={0};

void DrawFrame(int n);
Uint32 DrawFrame_callback(Uint32 interval, void *param);
void DrawCircle(float cx, float cy, float r, int num_segments) ;

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	
	window_w = displayMode.w;//*0.5;
	window_h = displayMode.h;//*0.5;
	
	window = SDL_CreateWindow(
							"Animators Place",
							 SDL_WINDOWPOS_CENTERED, 
							 SDL_WINDOWPOS_CENTERED, 
							 window_w, 
							 window_h, 
							 SDL_WINDOW_OPENGL
							 //|SDL_WINDOW_FULLSCREEN
							 |SDL_WINDOW_MAXIMIZED
							 //|SDL_WINDOW_RESIZABLE//must be for minimizing!
							 );
	SDL_GetWindowSize(window,&window_w,&window_h);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	//printf("%d\n",wglGetCurrentContext());
    glEnable(GL_TEXTURE_2D);
    
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    //(void)io;//wtf?

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();
    
    //ImVec4 clear_color = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    //ImVec4 menuColor = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    ImVec4 clear_color = ImVec4(1.f, 1.f, 1.f, 1.00f);
    ImVec4 menuColor = ImVec4(0.f, 0.f, 0.f, 1.00f);
    ImVec4 lightColor = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
    //window_flags |= ImGuiWindowFlags_NoBackground;
    //window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
    IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG);
    
    SDL_Surface *buttons[7];

    buttons[0] = IMG_Load("NewestDesign/Blank_Button.png");
    //buttons[1] = IMG_Load("NewestDesign/Logotype01.png");
    buttons[1] = IMG_Load("Icons/logo.png");
    buttons[2] = IMG_Load("NewestDesign/AddFrame_dark.png");
    buttons[3] = IMG_Load("NewestDesign/DeleteFrame_dark.png");
    buttons[4] = IMG_Load("NewestDesign/Play_dark.png");
    buttons[5] = IMG_Load("NewestDesign/Pause_dark.png");
    buttons[6] = IMG_Load("Icons/toonPlace_new.png");
    
    GLuint buttonsTextures[7];
    for (int i=0;i<7;i++)
    {
		glGenTextures(1, &buttonsTextures[i]);
		glBindTexture(GL_TEXTURE_2D, buttonsTextures[i]);

		//glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		if (i!=-1)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, buttons[i]->w, buttons[i]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buttons[i]->pixels);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buttons[i]->w, buttons[i]->h, 0, GL_RGB, GL_UNSIGNED_BYTE, buttons[i]->pixels);
		
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}   
	int state = 0;
	//ImFont *font = io.Fonts->AddFontFromFileTTF("12804.otf",window_h/10);
	ImFont *font = io.Fonts->AddFontFromFileTTF("Eremitage_Cyrillic.otf",window_h/10);
	ImFont *mediumFont = io.Fonts->AddFontFromFileTTF("Eremitage_Cyrillic.otf",window_h/20);
	ImFont *buttonsFont = io.Fonts->AddFontFromFileTTF("Eremitage_Cyrillic.otf",window_h/35);
    ImFont *smallFont = io.Fonts->AddFontFromFileTTF("Eremitage_Cyrillic.otf",window_h/40);
    ImFont *framesFont = io.Fonts->AddFontFromFileTTF("Eremitage_Cyrillic.otf",window_h/50);
    
    
	Uint32 ms;
	int FPS = 60;
	Uint32 MSPF = 1000/FPS;
		
    bool quit = false;
    const char *buttonsNames[5]={"New Toons","Popular","Editor","Wiki","Profile"};
    //vector<vector<point>> points;
    bool wasPressed=false;
    
    const Uint8 *keys;
    int oldPressZ = 0;
    
    
    layer tmpLayer;
    frame tmpFrame;

	tmpFrame.layers.push_back(tmpLayer);
	myToon.frames.push_back(tmpFrame);
	
	/*size_t*/int curFrame = 0;
    
	///*size_t*/int prevFrame = 0;
	///*size_t*/int prevPrevFrame = 0;
	
	myToon.frames[curFrame].curLayer = 0;
    frame copiedFrameBuffer;
    
    static int penSize = 1;
    
    
    float curColor[4]={0};
    
    int mx,my;
	unsigned int mouseState;
	
	SDL_TimerID my_timer_id;
	bool isPlaying=false;
	
	
	//SDL_GetWindowSize(window, &window_w, &window_h);
	menuHeight=window_h/10;
	menuWidth=window_w;
	
	vector<GLuint> renderedFrames(1);
	glGenTextures(1, &renderedFrames[0]);
	glBindTexture(GL_TEXTURE_2D, renderedFrames[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_w-window_w/3,window_h-menuHeight-window_h/8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);		
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			
	glBindTexture(GL_TEXTURE_2D, 0);
	
	SDL_SetWindowResizable(window,SDL_FALSE);
	
    while(!quit)
    {
		ms = SDL_GetTicks();
		keys = SDL_GetKeyboardState(NULL);
		mouseState = SDL_GetMouseState(&mx,&my);
		SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                quit = true;
            if (event.type == SDL_WINDOWEVENT)
            {
				/*if (event.window.event!=SDL_WINDOWEVENT_MINIMIZED)
				{
					needRedraw = true;
				}
				else
				{
					puts("Window collapsed");
				}*/
				if (event.window.event==SDL_WINDOWEVENT_RESIZED&&event.window.event!=SDL_WINDOWEVENT_MAXIMIZED)
				{
					
				}
				if (event.window.event==SDL_WINDOWEVENT_MINIMIZED)
				{
					puts("Window minimized");
					SDL_SetWindowResizable(window,SDL_TRUE);
				}

				if (event.window.event==SDL_WINDOWEVENT_RESTORED)
				{
					puts("Window restored");
					SDL_SetWindowResizable(window,SDL_FALSE);
				}
				if (event.window.event==SDL_WINDOWEVENT_MAXIMIZED)
				{
					puts("Window maximized");
				}
			}
			
			//if (state!=2) break;
			//if (isPlaying) break;
			//once time pressed
			if (event.type==SDL_KEYDOWN)
			{
				//puts("...");
				//printf("%d %d %d\n",event.key.keysym.scancode,SDLK_v,SDL_SCANCODE_V);
				switch(event.key.keysym.sym)
				{
					case SDLK_c: copiedFrameBuffer = myToon.frames[curFrame];break;
					case SDLK_v: 
						if (!wasPressed)
						{
							myToon.frames[curFrame] = copiedFrameBuffer;
							needRedraw = true;
						}
					break;
					case SDLK_z:
						if (myToon.frames[curFrame].layers[myToon.frames[curFrame].curLayer].lines.size()&&!wasPressed)
						{
							myToon.frames[curFrame].layers[myToon.frames[curFrame].curLayer].lines.pop_back();
							needRedraw=true;
						}
					break;
				}
			}
			
			//long time pressed
			/*if (keys[SDL_SCANCODE_Z])
			{
				if (event.key.timestamp-oldPressZ>85)
				{
					oldPressZ=event.key.timestamp;
					//puts("Z");
					if (myToon.frames[curFrame].layers[0].lines.size()&&!wasPressed)
					{
						myToon.frames[curFrame].layers[0].lines.pop_back();
						needRedraw=true;
					}
				}
			}*/
        }

        ImGui_ImplOpenGL2_NewFrame();
        
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding=ImVec2(0,0);
        style.WindowRounding = 0;
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		//ImGui::SetNextWindowSize(ImVec2(window_w/4,window_h));
		ImGui::SetNextWindowSize(ImVec2(menuWidth,menuHeight));
		
		ImGui::PushStyleColor(ImGuiCol_WindowBg,menuColor);
		ImGui::Begin("Menu",NULL, ImVec2(0, 0),-1,window_flags);
		
		//here widgets
		
		//ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "This is some useful text.");
		ImGui::SetCursorPos(ImVec2(menuHeight*0.125,menuHeight*0.125));
		ImGui::Image((void*)(intptr_t)buttonsTextures[1],ImVec2(menuHeight*0.75,menuHeight*0.75));
		for (int i=0;i<5;i++)
		{
			ImGui::SameLine();
			
			//ImGui::SetCursorPos(ImVec2(window_w/4/2-window_w/5/2,window_h/64+1.5*window_w/4/4+i*window_h/10+i*window_h/64));
			ImGui::SetCursorPos(ImVec2(menuHeight*1.25+i*menuWidth/10+i*menuWidth/30,menuHeight/2-menuHeight/4));
			
			ImGui::PushID(buttonsNames[i]);
			if (ImGui::ImageButton((void*)(intptr_t)buttonsTextures[0],ImVec2(window_w/10,menuHeight/2),ImVec2(0,0),ImVec2(1,1),0,menuColor))
			//if (ImGui::InvisibleButton("",ImVec2(window_w/10,menuHeight/2)))
			{
				state = i;
				needRedraw = true;
			}
			ImGui::PopID();
			
			
			if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
			{
				ImGui::SetMouseCursor(7);
				//ImGui::SetCursorPos(ImVec2(window_w/4/2-1.1*window_w/5/2, window_h/64+1.5*window_w/4/4+i*window_h/10+i*window_h/64-window_h/128));
				//ImGui::ImageButton((void*)(intptr_t)buttonsTextures[0],ImVec2(1.1*window_w/10,1.1*menuHeight),ImVec2(0,0),ImVec2(1,1),0,menuColor);
			}
			//ImGui::SetCursorPosX((window_w/4/2-window_w/5/2));
			//ImGui::SetCursorPosY((window_h/64+1.5*window_w/4/4+i*window_h/10+i*window_h/64));
			
			//printf("???: %lf\n",(window_w/10-ImGui::CalcTextSize(buttonsNames[i]).x)/2);
			ImGui::PushFont(buttonsFont);
			//printf("???: %lf\n",ImGui::CalcTextSize(buttonsNames[i]).x);
			
			ImGui::SetCursorPos(ImVec2(menuHeight*1.25+(window_w/10-ImGui::CalcTextSize(buttonsNames[i]).x)/2+i*menuWidth/10+i*menuWidth/30,menuHeight/2-menuHeight/8));
			
			
			ImGui::TextColored(ImVec4(1.0f,1.0f,1.0f,1.0f), buttonsNames[i]);
			ImGui::PopFont();
			
		}
		
		ImGui::End();
		ImGui::PopStyleColor();
		
		switch (state)
		{
			case 0://New
			{
				
				ImGui::SetNextWindowPos(ImVec2(0,menuHeight));
				ImGui::SetNextWindowSize(ImVec2(window_w/*-window_w/3*/,window_h-menuHeight));

				ImGui::PushStyleColor(ImGuiCol_WindowBg,clear_color);
		
				ImGui::Begin("Main",NULL, ImVec2(0, 0),-1,window_flags);
					ImGui::SetCursorPosX(((window_w)-ImGui::CalcTextSize("New Toons").x)/2);
					ImGui::PushFont(font);
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "New Toons");
					ImGui::PopFont();
					//ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
					
					for (int i=0;i<3;i++)
					{
						for (int j=0;j<4;j++)
						{
							if(j==0)
								ImGui::SetCursorPosX((window_w-4*4*window_w/20)/2);
							ImGui::PushID(i*4+j);
							if (ImGui::ImageButton((void*)(intptr_t)buttonsTextures[6],ImVec2(4*window_w/20,3*window_w/20),ImVec2(0,0),ImVec2(1,1),0,clear_color))
							{
								state = 5;
							}
							ImGui::PopID();
							if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
							{
								ImGui::SetMouseCursor(7);
							}
							if (j<3)
								ImGui::SameLine();
						}
					}
            
				ImGui::End();
				ImGui::PopStyleColor();
				
				//ImGui::SetNextWindowPos(ImVec2(window_w-window_w/3, 0));
				//ImGui::SetNextWindowSize(ImVec2(window_w/3,window_h));
				//ImGui::Begin("TrumpPlace",NULL, ImVec2(0, 0),-1,window_flags);
					//ImGui::ImageButton((void*)(intptr_t)buttonsTextures[5],ImVec2(window_w/3,window_h/3),ImVec2(0,0),ImVec2(1,1),0,clear_color);
					//ImGui::ImageButton((void*)(intptr_t)buttonsTextures[6],ImVec2(window_w/3,window_h/3),ImVec2(0,0),ImVec2(1,1),0,clear_color);
					
				//ImGui::End();
			}break;
			case 1://Popular
			{
				ImGui::SetNextWindowPos(ImVec2(0, menuHeight));
				ImGui::SetNextWindowSize(ImVec2(window_w,window_h-menuHeight));
				ImGui::PushStyleColor(ImGuiCol_WindowBg,clear_color);
				ImGui::Begin("Main",NULL, ImVec2(0, 0),-1,window_flags);
					ImGui::SetCursorPosX(((window_w)-ImGui::CalcTextSize("Popular Toons").x)/2);
					ImGui::PushFont(font);
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "Popular Toons");
					ImGui::PopFont();
					for (int i=0;i<3;i++)
					{
						for (int j=0;j<4;j++)
						{
							if(j==0)
								ImGui::SetCursorPosX((window_w-4*4*window_w/20)/2);
							ImGui::PushID(i*4+j);
							if (ImGui::ImageButton((void*)(intptr_t)buttonsTextures[6],ImVec2(4*window_w/20,3*window_w/20),ImVec2(0,0),ImVec2(1,1),0,clear_color))
							{
								state = 5;
							}
							ImGui::PopID();
							if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
							{
								ImGui::SetMouseCursor(7);
							}
							if (j<3)
								ImGui::SameLine();
						}
					}
				ImGui::End();
				ImGui::PopStyleColor();
				//ImGui::SetNextWindowPos(ImVec2(window_w-window_w/3, 0));
				//ImGui::SetNextWindowSize(ImVec2(window_w/3,window_h));
				//ImGui::Begin("TrumpPlace",NULL, ImVec2(0, 0),-1,window_flags);
				//ImGui::End();
			}break;
			case 2://Editor
			{
				
				ImGui::SetNextWindowPos(ImVec2(window_w-window_w/3, menuHeight));
				ImGui::SetNextWindowSize(ImVec2(window_w/3,(window_h-menuHeight)/3));
				ImGui::PushStyleColor(ImGuiCol_WindowBg,lightColor);
				ImGui::Begin("Layers",NULL, ImVec2(0, 0),-1,window_flags);
				
				ImGui::End();
				ImGui::PopStyleColor();
				
				ImGui::SetNextWindowPos(ImVec2(window_w-window_w/3, menuHeight+(window_h-menuHeight)/3));
				ImGui::SetNextWindowSize(ImVec2(window_w/3,window_h-menuHeight-(window_h-menuHeight)/3));
				ImGui::PushStyleColor(ImGuiCol_WindowBg,lightColor);
				//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(20,0));
				ImGui::Begin("Right",NULL, ImVec2(0, 0),-1,window_flags);
					ImGui::PushFont(smallFont);
					//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(20,0));
					//ImGui::SetCursorPosX(20);
					ImGui::SliderInt("Pen Size",&penSize,1,100);
					if (penSize<1) penSize = 1;
					else if (penSize>200) penSize=200;
					
					//ImGui::SetCursorPosX(20);
					if (ImGui::SliderInt("Frames",&curFrame,0,myToon.frames.size()-1))
					{
						needRedraw = true;
					}
					if (curFrame<0)
					{
						curFrame=0;
					}
					else if ((size_t)curFrame>myToon.frames.size()-1)
					{
						curFrame = myToon.frames.size()-1;
					}
					
					//ImGui::SetCursorPosX(20);
					ImGui::ColorEdit4("Color",curColor);
					/*if (ImGui::IsItemEdited()||ImGui::IsItemDeactivated()&&!ImGui::IsItemHovered())
					{
						puts("test");
						//needRedraw = true;
					}*/
					/*if (ImGui::ColorEdit4("Color",curColor))
					{
						needRedraw = true;
					}*/
					ImGui::PopFont();
					//ImGui::PopStyleVar();
				ImGui::End();
				ImGui::PopStyleColor();
				//ImGui::PopStyleVar();
				
				ImGui::SetNextWindowPos(ImVec2(0, window_h-window_h/8));
				ImGui::SetNextWindowSize(ImVec2(window_w/13/*window_w-window_w/3*/,window_h/8));
				ImGui::PushStyleColor(ImGuiCol_WindowBg,lightColor);
				ImGui::Begin("BottomLeft",NULL, ImVec2(0, 0),-1,window_flags);
					//Add
					if (ImGui::ImageButton((void*)(intptr_t)buttonsTextures[2],ImVec2(window_w/30,window_w/30),ImVec2(0,0),ImVec2(1,1),0,lightColor))
					{
						layer tmpLayer;
						frame tmpFrame;
						tmpFrame.layers.push_back(tmpLayer);
						curFrame++;
						myToon.frames.insert(myToon.frames.begin()+curFrame,tmpFrame);
						myToon.frames[curFrame].curLayer = 0;
						needRedraw = true;
						
						GLuint tmpTex;
						//size_t frameNum = renderedFrames.size();
						renderedFrames.insert(renderedFrames.begin()+curFrame,tmpTex);
						glGenTextures(1, &renderedFrames[curFrame]);
						glBindTexture(GL_TEXTURE_2D, renderedFrames[curFrame]);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_w-window_w/3,window_h-menuHeight-window_h/8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);		
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );		
						glBindTexture(GL_TEXTURE_2D, 0);
					}
					if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
					{
						ImGui::SetMouseCursor(7);
					}
					ImGui::SameLine();
					
					//Delete
					if (ImGui::ImageButton((void*)(intptr_t)buttonsTextures[3],ImVec2(window_w/30,window_w/30),ImVec2(0,0),ImVec2(1,1),0,lightColor))
					{
						if (myToon.frames.size()>1)
						{
							myToon.frames.erase(myToon.frames.begin()+curFrame);
							renderedFrames.erase(renderedFrames.begin()+curFrame);
							if (curFrame>0)
								curFrame--;
							needRedraw = true;
						}
					}
					if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
					{
						ImGui::SetMouseCursor(7);
					}
					
					
					//Play/Pause
					if (ImGui::ImageButton(isPlaying?((void*)(intptr_t)buttonsTextures[5]):((void*)(intptr_t)buttonsTextures[4]),ImVec2(window_w/30,window_w/30),ImVec2(0,0),ImVec2(1,1),0,lightColor))
					{
						if (!isPlaying)
						{
							playingFrame = 0;
							//SDL_GL_MakeCurrent(window,NULL);
							my_timer_id = SDL_AddTimer(100, DrawFrame_callback, &gl_context);
							
							isPlaying = true;
							needRedraw = true;
							puts("start");
						}
						else
						{
							SDL_RemoveTimer(my_timer_id);
							isPlaying = false;
							needRedraw = true;
							puts("stop");
						}
					}
					if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
					{
						ImGui::SetMouseCursor(7);
					}
				ImGui::End();
				ImGui::PopStyleColor();
				
				ImGui::SetNextWindowPos(ImVec2(window_w/13, window_h-window_h/8));
				ImGui::SetNextWindowSize(ImVec2(window_w-window_w/3-window_w/13,window_h/8));
				ImGui::PushStyleColor(ImGuiCol_WindowBg,lightColor);
				ImGui::PushStyleColor(ImGuiCol_ScrollbarBg,ImVec4(0.4f,0.4f,0.4f,1.f));
				ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize,window_h/8/5);
				ImGui::Begin("BottomCenter",NULL, ImVec2(0, 0),-1,window_flags);
					//MiniFrames
					for (size_t i=0;i<myToon.frames.size();i++)
					{
						ImGui::SameLine();
						ImGui::SetCursorPos(ImVec2(i*(/*window_w/15*/(window_h/8*4/5)*4/3+3),0/*(window_h/8-3*window_w/60-10)/2*/));
						ImGui::PushID(i);
						ImGui::PushStyleColor(ImGuiCol_Border,i==(size_t)curFrame?ImVec4(1.f,0.f,0.f,1.f):ImVec4(0.f,0.f,0.f,1.f));
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,3);
						if (ImGui::ImageButton((void*)(intptr_t)renderedFrames[i]/*buttonsTextures[4]*/,ImVec2((window_h/8*4/5)*4/3,window_h/8*4/5/*4*window_w/60,3*window_w/60*/),ImVec2(0,1),ImVec2(1,0),0,menuColor))
						{
							curFrame=i;
							needRedraw = true;
						}
						ImGui::PopStyleVar();
						ImGui::PopStyleColor();
						ImGui::PopID();
						const char *s=to_string(i+1).data();
						
						ImGui::SetCursorPos(ImVec2(i*(/*window_w/15*/(window_h/8*4/5)*4/3+3),2));
						ImGui::PushFont(framesFont);
						ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), s);
						ImGui::PopFont();
					}
					
				
				ImGui::End();
				ImGui::PopStyleColor(2);
				ImGui::PopStyleVar();
				/*if (event.type==SDL_MOUSEBUTTONDOWN&&event.button.button == SDL_BUTTON_LEFT)
				{
					printf("%d %d\n",mx,my);
				}*/
				
				if (isPlaying)
				{
					ImGui::SetNextWindowPos(ImVec2(0, menuHeight));
					ImGui::SetNextWindowSize(ImVec2(window_w-window_w/3,window_h-menuHeight-window_h/8));
					ImGui::PushStyleColor(ImGuiCol_WindowBg,lightColor);
					ImGui::Begin("Center",NULL, ImVec2(0, 0),-1,window_flags);
						//if (isPlaying)
							ImGui::ImageButton((void*)renderedFrames[playingFrame], ImVec2(window_w-window_w/3,window_h-menuHeight-window_h/8),ImVec2(0,1),ImVec2(1,0),0,ImVec4(1,1,1,1));
						//else
						//	ImGui::ImageButton((void*)renderedFrames[curFrame], ImVec2(window_w-window_w/3,window_h-menuHeight-window_h/8),ImVec2(0,0),ImVec2(1,1),0,ImVec4(1,1,1,1));
						
					ImGui::End();
					ImGui::PopStyleColor();
				}
				
				if (!isPlaying)
				{
					//if (1/*mx>=0&&mx<=window_w-window_w/3&&
					//	my>=0&&my<=window_h-window_h/8*/)
					//{
						//printf("%d %d\n",mx,my);
						//int mx,my;
						//unsigned int mouseState = SDL_GetMouseState(&mx,&my);
						point mp;
						mp.x=ImGui::GetIO().MousePos.x;
							//mx;
						mp.y=ImGui::GetIO().MousePos.y;
							//my;
						//needRedraw=true;//too stupid...

						//if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))/*&&(!wasPressed)*/)
						if (ImGui::IsMouseDown(0))
						//if (io.MouseDownDuration[0] >= 0.0f)
						{
							//puts("Mouse left click");
							if (ImGui::IsMouseClicked(0)&&!wasPressed&&mx>=0&&mx<=window_w-window_w/3&&
							my>=menuHeight&&my<=window_h-window_h/8)
							{
								
								//puts("first pressed");
								//printf("%d %d\n",mx,my);
								wasPressed = true;
								//points.push_back(vector<point>(0));

								line tmpLine;
								tmpLine.points = vector<point>(0);
								tmpLine.width = penSize;
								tmpLine.r = curColor[0];
								tmpLine.g = curColor[1];
								tmpLine.b = curColor[2];
								tmpLine.a = curColor[3];
								
								myToon.frames[curFrame].layers[0].lines.push_back(tmpLine);

							}
						}
						else
						{
							wasPressed = false;
						}
						
						if (wasPressed)
						{
							//points[points.size()-1].push_back(mp);
							//if (event.type==SDL_MOUSEMOTION)//doesnt work ;/
								myToon.frames[curFrame].layers[0].lines[myToon.frames[curFrame].layers[0].lines.size()-1].points.push_back(mp);
						}

					//}
					//printf("%u %u\n",event.type,SDL_KEYDOWN);
					//printf("%u %u\n",event.key.keysym.sym,SDLK_z);
					
					
					
					if (keys[SDL_SCANCODE_A]&&!wasPressed)
					{
						if (curFrame>0)
							curFrame--;
						/*else
						{
							curFrame=myToon.frames.size()-1;
						}*/
						needRedraw = true;
					}
					if (keys[SDL_SCANCODE_D]&&!wasPressed)
					{
						if ((size_t)curFrame+1<myToon.frames.size())
							curFrame++;
						/*else
						{
							curFrame=0;
						}*/
						needRedraw = true;
					}
				}
				
			}break;
			case 3://Wiki?
			{
				ImGui::SetNextWindowPos(ImVec2(0,menuHeight));
				ImGui::SetNextWindowSize(ImVec2(window_w/*-window_w/3*/,window_h-menuHeight));

				ImGui::PushStyleColor(ImGuiCol_WindowBg,clear_color);
		
				ImGui::Begin("Main",NULL, ImVec2(0, 0),-1,window_flags);
					ImGui::SetCursorPosX(((window_w)-ImGui::CalcTextSize("Wiki").x)/2);
					ImGui::PushFont(font);
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "Wiki");
					ImGui::PopFont();
				ImGui::End();
				ImGui::PopStyleColor();
			}break;
			case 4://Profile
			{
				ImGui::SetNextWindowPos(ImVec2(0,menuHeight));
				ImGui::SetNextWindowSize(ImVec2(window_w/*-window_w/3*/,window_h-menuHeight));
				
				ImGui::PushStyleColor(ImGuiCol_WindowBg,clear_color);
		
				ImGui::Begin("Main",NULL, ImVec2(0, 0),-1,window_flags);
					ImGui::SetCursorPosX(((window_w)-ImGui::CalcTextSize("Profile").x)/2);
					ImGui::PushFont(font);
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "Profile");
					ImGui::PopFont();
					
					ImGui::Image((void*)(intptr_t)buttonsTextures[1],ImVec2(4*window_w/20,3*window_w/20),ImVec2(0,1),ImVec2(1,0),menuColor);
					ImGui::SameLine();
					ImGui::SetCursorPosX(4*window_w/20+window_w/20);
					ImGui::PushFont(mediumFont);
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "Nick: ");
					ImGui::PopFont();
					
					ImGui::PushFont(mediumFont);
					ImGui::SetCursorPosX((window_w-(ImGui::CalcTextSize("Album").x+ImGui::CalcTextSize("Favorites").x+ImGui::CalcTextSize("Drafts").x+ImGui::CalcTextSize("Comments").x))/2);
					
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "Album");ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "Favorites");ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "Drafts");ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "Comments");
					ImGui::PopFont();
					
					for (int i=0;i<3;i++)
					{
						for (int j=0;j<4;j++)
						{
							if(j==0)
								ImGui::SetCursorPosX((window_w-4*4*window_w/20)/2);
							ImGui::PushID(i*4+j);
							if (ImGui::ImageButton((void*)(intptr_t)buttonsTextures[6],ImVec2(4*window_w/20,3*window_w/20),ImVec2(0,0),ImVec2(1,1),0,clear_color))
							{
								state = 5;
							}
							ImGui::PopID();
							if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
							{
								ImGui::SetMouseCursor(7);
							}
							if (j<3)
								ImGui::SameLine();
						}
					}
				
				ImGui::End();
				ImGui::PopStyleColor();
			}break;
			case 5://View
			{
				ImGui::SetNextWindowPos(ImVec2(0,menuHeight));
				ImGui::SetNextWindowSize(ImVec2(window_w/*-window_w/3*/,window_h-menuHeight));

				ImGui::PushStyleColor(ImGuiCol_WindowBg,clear_color);
		
				ImGui::Begin("Main",NULL, ImVec2(0, 0),-1,window_flags);
					ImGui::SetCursorPosX(((window_w)-ImGui::CalcTextSize("View").x)/2);
					ImGui::PushFont(font);
					ImGui::TextColored(ImVec4(0.0f,0.0f,0.0f,1.0f), "View");
					ImGui::PopFont();
				ImGui::End();
				ImGui::PopStyleColor();
			}
		}
		
        ImGui::Render();
        if (state==2&&!isPlaying)
		{
			//printf("%u\n",curFrame);
			glPointSize(1);
			//if (!isPlaying)
			//{
				glViewport(0, 0, window_w, window_h);
				
				//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
				
				glClearColor(1,1,1,1);
				if (needRedraw)
				{
					glClear(GL_COLOR_BUFFER_BIT);
				}

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				gluOrtho2D( 0.0, window_w, window_h,0.0 );
				//printf("before frame %d\n",SDL_GetTicks()-ms);
				
				//if need full Render
				//else{
				//Draw old texture
				//Render new points}
				//Copy new texture
				//Draw circle
				
				if (isPlaying)
				{
					//DrawFrame(playingFrame);
				}
				else
				{
					DrawFrame(curFrame);
				}
				
				if (!isPlaying)
				{
					glBindTexture(GL_TEXTURE_2D, renderedFrames[curFrame]);
					glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,window_h/8,window_w-window_w/3,window_h-menuHeight-window_h/8);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				//printf("after frame %d\n",SDL_GetTicks()-ms);
				
			//}
			
		}//if (state==2)
        
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		
		glColor4f(curColor[0],curColor[1],curColor[2],curColor[3]);
		//DrawCircle(mx, my, penSize/2,penSize);
		SDL_GL_SwapWindow(window);
		//printf("after all %d\n",SDL_GetTicks()-ms);

        ms = SDL_GetTicks()-ms;
        //printf("%d %d\n",ms,MSPF);
		if (ms < MSPF)
			SDL_Delay(MSPF-ms);
	}
    
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
	return 0;
}


void DrawFrame(int n)
{
	int curFrame = n;/*playingFrame*/ //*(int*)param;
	
	size_t l=myToon.frames[curFrame].curLayer;
	size_t i;
	if (needRedraw)
	{
		i=0;
		needRedraw = false;
	}
	else
	{
		i=myToon.frames[curFrame].layers[l].lines.size()-1;
	}
	
	for (;i<myToon.frames[curFrame].layers[l].lines.size();i++)
	{
		//glEnable(GL_POINT_SMOOTH);
		
		//printf("i = %u\n",i);
		//glBegin(GL_LINES);
		
		/****Generate a circle****/
		vector<point> tmpCircle;
		point tmpPoint;
		
		//taken from here: http://slabode.exofire.net/circle_draw.shtml
		float theta = 2 * 3.1415926 / (float)myToon.frames[curFrame].layers[l].lines[i].width; 
		float tangetial_factor = tanf(theta);//calculate the tangential factor 

		float radial_factor = cosf(theta);//calculate the radial factor 
		
		float x = (float)myToon.frames[curFrame].layers[l].lines[i].width/2.;//we start at angle = 0 

		float y = 0; 

		for(int ii = 0; ii < myToon.frames[curFrame].layers[l].lines[i].width+15; ii++) 
		{ 
			//glVertex2f(x + cx, y + cy);//output vertex 
			tmpPoint.x=x;//round(x);
			tmpPoint.y=y;//round(y);
			tmpCircle.push_back(tmpPoint);
			
			//calculate the tangential vector 
			//remember, the radial vector is (x, y) 
			//to get the tangential vector we flip those coordinates and negate one of them 

			float tx = -y; 
			float ty = x; 
			
			//add the tangential vector 

			x += tx * tangetial_factor; 
			y += ty * tangetial_factor; 
			
			//correct using the radial factor 

			x *= radial_factor; 
			y *= radial_factor; 
		}
		
		size_t j;
		/*if (needRedraw)
		{
			j=0;
		}
		else
		{
			j=myToon.frames[curFrame].layers[l].lines[i].points.size()-2;
		}*/
		
		for (j=0;j+1<myToon.frames[curFrame].layers[l].lines[i].points.size();j++)
		{
			//glVertex2i(points[i][j].x/**window_w/500*/,points[i][j].y/**window_h/300*/);
			//glVertex2i(points[i][j+1].x/**window_w/500*/,points[i][j+1].y/**window_h/300*/);
			glColor4f(myToon.frames[curFrame].layers[l].lines[i].r,
			  myToon.frames[curFrame].layers[l].lines[i].g,
			  myToon.frames[curFrame].layers[l].lines[i].b,
			  myToon.frames[curFrame].layers[l].lines[i].a);
			float lineLength = sqrt(
				pow(myToon.frames[curFrame].layers[l].lines[i].points[j].x-
					myToon.frames[curFrame].layers[l].lines[i].points[j+1].x
					,2)
				+
				pow(myToon.frames[curFrame].layers[l].lines[i].points[j].y-
					myToon.frames[curFrame].layers[l].lines[i].points[j+1].y
					,2)
				);
			
			float rotAngle = atan(
				(myToon.frames[curFrame].layers[l].lines[i].points[j+1].y-
				myToon.frames[curFrame].layers[l].lines[i].points[j].y)
				/
				(myToon.frames[curFrame].layers[l].lines[i].points[j+1].x-
				myToon.frames[curFrame].layers[l].lines[i].points[j].x)
				) * 180/M_PI;
			if (lineLength<1e-5)
				rotAngle=0;
			//printf("len=%f angle=%f\n",lineLength,rotAngle);
			/*
			glBegin(GL_POINTS);
				glVertex2i(myToon.frames[0].layers[l].lines[i].points[j].x,myToon.frames[0].layers[l].lines[i].points[j].y);
				glVertex2i(myToon.frames[0].layers[l].lines[i].points[j+1].x,myToon.frames[0].layers[l].lines[i].points[j+1].y);
			glEnd();
			*/
			glTranslatef(myToon.frames[curFrame].layers[l].lines[i].points[j].x,
				myToon.frames[curFrame].layers[l].lines[i].points[j].y,
				0);
			if(myToon.frames[curFrame].layers[l].lines[i].width>1)
			{
			glBegin(GL_TRIANGLE_FAN);
				for (size_t p=0;p+1<tmpCircle.size();p++)
				{
					glVertex2f(tmpCircle[p].x,tmpCircle[p].y);
					//glVertex2f(tmpCircle[p].x,tmpCircle[p].y);
				}
			glEnd();
			}
			//DrawCircle(0, 0, myToon.frames[curFrame].layers[l].lines[i].width/2.,myToon.frames[curFrame].layers[l].lines[i].width);
			glTranslatef(-myToon.frames[curFrame].layers[l].lines[i].points[j].x,
				-myToon.frames[curFrame].layers[l].lines[i].points[j].y,
				0);
			
			glTranslatef(myToon.frames[curFrame].layers[l].lines[i].points[j+1].x,
				myToon.frames[curFrame].layers[l].lines[i].points[j+1].y,
				0);
			if(myToon.frames[curFrame].layers[l].lines[i].width>1)
			{
			glBegin(GL_TRIANGLE_FAN);
				for (size_t p=0;p+1<tmpCircle.size();p++)
				{
					glVertex2f(tmpCircle[p].x,tmpCircle[p].y);
				}
			glEnd();
			}
			//DrawCircle(0, 0, myToon.frames[curFrame].layers[l].lines[i].width/2.,myToon.frames[curFrame].layers[l].lines[i].width);
			glTranslatef(-myToon.frames[curFrame].layers[l].lines[i].points[j+1].x,
				-myToon.frames[curFrame].layers[l].lines[i].points[j+1].y,
				0);
			
			if (myToon.frames[curFrame].layers[l].lines[i].points[j+1].x>=
		myToon.frames[curFrame].layers[l].lines[i].points[j].x)
			{
				glTranslatef(myToon.frames[curFrame].layers[l].lines[i].points[j].x+
						myToon.frames[curFrame].layers[l].lines[i].width/2*cos((rotAngle-90)/180*M_PI),
					myToon.frames[curFrame].layers[l].lines[i].points[j].y+
						myToon.frames[curFrame].layers[l].lines[i].width/2*sin((rotAngle-90)/180*M_PI),
					0);
			}
			else
			{
				glTranslatef(myToon.frames[curFrame].layers[l].lines[i].points[j+1].x+
						myToon.frames[curFrame].layers[l].lines[i].width/2*cos((rotAngle-90)/180*M_PI),
					myToon.frames[curFrame].layers[l].lines[i].points[j+1].y+
						myToon.frames[curFrame].layers[l].lines[i].width/2*sin((rotAngle-90)/180*M_PI),
					0);
			}
			glRotatef(rotAngle, 0.0f, 0.0f, 1.0f);
			
			//Here is bug with odd width. Now its stupid fix "-myToon.frames[curFrame].layers[l].lines[i].width%2"*/
			glBegin(GL_QUAD_STRIP);
				glVertex2f(0,0);
				glVertex2f(0,myToon.frames[curFrame].layers[l].lines[i].width);/*>1?
					myToon.frames[curFrame].layers[l].lines[i].width-myToon.frames[curFrame].layers[l].lines[i].width%2
					:myToon.frames[curFrame].layers[l].lines[i].width);*/
				glVertex2f(lineLength,0);
				glVertex2f(lineLength,myToon.frames[curFrame].layers[l].lines[i].width);/*>1?
					myToon.frames[curFrame].layers[l].lines[i].width-myToon.frames[curFrame].layers[l].lines[i].width%2
					:myToon.frames[curFrame].layers[l].lines[i].width);*/
			glEnd();
			
			glRotatef(-rotAngle, 0.0f, 0.0f, 1.0f);
			if (myToon.frames[curFrame].layers[l].lines[i].points[j+1].x>=
		myToon.frames[curFrame].layers[l].lines[i].points[j].x)
			{
		glTranslatef(-(myToon.frames[curFrame].layers[l].lines[i].points[j].x+
				myToon.frames[curFrame].layers[l].lines[i].width/2*cos((rotAngle-90)/180*M_PI)),
			-(myToon.frames[curFrame].layers[l].lines[i].points[j].y+
				myToon.frames[curFrame].layers[l].lines[i].width/2*sin((rotAngle-90)/180*M_PI)),
			0);
			}
			else
			{
		glTranslatef(-(myToon.frames[curFrame].layers[l].lines[i].points[j+1].x+
				myToon.frames[curFrame].layers[l].lines[i].width/2*cos((rotAngle-90)/180*M_PI)),
			-(myToon.frames[curFrame].layers[l].lines[i].points[j+1].y+
				myToon.frames[curFrame].layers[l].lines[i].width/2*sin((rotAngle-90)/180*M_PI)),
			0);
			}
			//for mouse
			/*glTranslatef(mx,my,0);
			glBegin(GL_LINES);
			
			for (size_t p=0;p+1<tmpCircle.size();p++)
			{
		glVertex2f(tmpCircle[p].x,tmpCircle[p].y);
			}
			glEnd();
			glTranslatef(-mx,-my,0);
			*/
		}//for j
		
	}//for i
	//needRedraw = false;
}

Uint32 DrawFrame_callback(Uint32 interval, void *param)
{
	needRedraw = true;
	printf("%d frame drown\n",playingFrame);
	if ((size_t)playingFrame+1<myToon.frames.size())
		playingFrame++;
	else
		playingFrame = 0;
	
	return interval;
}

void DrawCircle(float cx, float cy, float r, int num_segments) 
{ 
	//taken from here: http://slabode.exofire.net/circle_draw.shtml
	float theta = 2 * 3.1415926 / float(num_segments); 
	float tangetial_factor = tanf(theta);//calculate the tangential factor 

	float radial_factor = cosf(theta);//calculate the radial factor 
	
	float x = r;//we start at angle = 0 

	float y = 0; 
    
	glBegin(/*GL_LINE_LOOP*/GL_TRIANGLE_FAN); 
	for(int ii = 0; ii < num_segments; ii++) 
	{ 
		glVertex2f(x + cx, y + cy);//output vertex 
        
		//calculate the tangential vector 
		//remember, the radial vector is (x, y) 
		//to get the tangential vector we flip those coordinates and negate one of them 

		float tx = -y; 
		float ty = x; 
        
		//add the tangential vector 

		x += tx * tangetial_factor; 
		y += ty * tangetial_factor; 
        
		//correct using the radial factor 

		x *= radial_factor; 
		y *= radial_factor; 
	} 
	glEnd(); 
}
