
#include "Dx12Core/Application.h"

using namespace Dx12Core;

class TriangleApp : public ApplicationDx12Base
{
public:
	TriangleApp() = default;

protected:
	void Update(double elapsedTime) override {};
	void Render() override {};
};


CREATE_APPLICATION(TriangleApp)