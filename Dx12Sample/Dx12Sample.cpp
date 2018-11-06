#include "stdafx.h"
#include "Dx12SampleApp.h"

using namespace Dx12Sample;
using namespace Forms;

int main()
{
	Application::Init(GetModuleHandle(nullptr));
	Application::EnableVisualStyles();
	Form *form = new Form(std::wstring(L"Test D3D12"));
	form->InitComponent();
	Dx12SampleApp app(form);
	app.Init();
	Application::Run(form);
	delete form;
    return 0;
}
