#include "Application.h"

void Application::Run()
{

}

bool Application::Init() {
	return false;
}

void Application::Terminate() {

}

Application::Application() {

}

Application::~Application() {

}

Application& Application::Instance() {
	static Application instance;
	return instance;
}