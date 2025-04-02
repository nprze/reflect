#include "world.h"

rfct::world rfct::world::currentWorld;

void rfct::world::loadScene(const std::string& path)
{
	m_currentScene = new scene();
	m_currentScene->loadScene(path);
}

void rfct::world::cleanWorld() 
{ 
	m_currentScene->unloadScene(); 
	delete m_currentScene; 
}
