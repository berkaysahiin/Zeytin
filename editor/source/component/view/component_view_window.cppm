module;

#include <memory>

export module zeytin.component.view;

export class ComponentViewWindow {
public:
	ComponentViewWindow();
	~ComponentViewWindow();
	
	void render();
	
private:
	struct Impl;
	std::unique_ptr<Impl> pImpl;
};
