#pragma once
#define _CRTDBG_MAP_ALLOC  // works with the 'look for memory leaks' thing below
#include <iostream> // Or any other standard library headers
#include <stdlib.h>
#include <crtdbg.h>
#include "Debug.h"
#include <string>

#include "MemoryMonitor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


#include "SceneManager.h"


#include "MMath.h"
using namespace MATH;
 
int main(int argc, char* args[]) {
	/// Look for memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	
#pragma warning(disable : 4996) 
	/*if (const char* env_p = std::getenv("PATH")) {
		std::cout << "Your PATH is: " << env_p << '\n';
	}*/

	static_assert(sizeof(int*) == 8, "Vulkan no longer supports x86 builds"); //means "close the program right now" if there's an error, won't even let me build
	// basically can't change to x86

	Debug::DebugInit("Vulkan Graphics Engine Log");
	Debug::Info("Starting the GameSceneManager", __FILE__, __LINE__);
	SceneManager* gsm = new SceneManager();
	if (gsm->Initialize("Vulkan Graphics Engine", 1280, 720) ==  true) {
		gsm->Run();
	} 
	delete gsm;
	exit(0);
	writeMemoryLog();
}