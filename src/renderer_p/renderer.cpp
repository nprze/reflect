#include "C:/Users/Natal/Documents/games/reflect/CMakeFiles/reflect.dir/Debug/cmake_pch.hxx"
#include "renderer.h"
rfct::renderer rfct::renderer::ren;
rfct::renderer::renderer()
	: m_window(1000, 500, "reflect"), m_instance(), m_device()
{
}

void rfct::renderer::run()
{
	m_window.show();
	while (m_window.pollEvents());
}
