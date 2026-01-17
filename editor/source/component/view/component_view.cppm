module;

#include <memory>

export module zeytin.component.view;

export class ComponentView {
public:
	ComponentView();
	~ComponentView();
	
	void render();
	
private:
	struct Impl;
	std::unique_ptr<Impl> pImpl;
};
