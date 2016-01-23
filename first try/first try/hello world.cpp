#include <Windows.h>  // Need to be included so i can use windows functions. 
void initialiseWindow();  // decleration for initialisewindow
void Run();    // decleration for the run file. 
LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam); // unsecure what this is :/

void main()
{
	initialiseWindow();
	Run();
}



void initialiseWindow()
{
	HINSTANCE applicationHandle = GetModuleHandle(NULL);  // GetModulehandle returns a handle if the parameter is Null and I return a handle to the applicationHandle. basiclly windows knows if you run multiple copys of a program witch copy is witch.
	WNDCLASS windowClass; // definiton for the window
	windowClass.style = CS_HREDRAW | CS_VREDRAW;  // CS_HREDRAW = Redraws the entire window if a movement or size adjustment changes the width of the client area.   CS_VREDRAW = Redraws the entire window if a movement or size adjustment changes the height of the client area.
	windowClass.lpfnWndProc = WindowProcedure; // This is a pointer to the call back funciton from above. Dont really know why i need it or what the function up above does.
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = applicationHandle;


}