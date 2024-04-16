#pragma once

struct EngineCreateInfo
{
	struct
	{
		int width = 1024;
		int height = 768;
		const wchar_t* title = L"Game";
	} window;
};

struct EngineAppImpl;

class EngineApp
{
public:
	EngineApp();
	virtual ~EngineApp();

	void Run();

	virtual EngineCreateInfo GetCreateInfo() const { return {}; }
	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnFrame() = 0;

	void Exit();
	float GetDeltaTime() const;

private:
	bool create();
	void destroy();
	void frame();
	bool IsEnd() const;

	EngineAppImpl* m_data;
};