#pragma once
#include "ObjLoader.h"

class Body
{
public:
	Body() {}
	~Body() {}

	//returns true if collided
	bool CheckCollision(glm::vec2 pos) {
		for (int i(0); i < chain.size(); ++i) {
			if (pos == chain[i])
				return true;
		}
		return false;
	}
	unsigned GetSize() { return chain.size() + toAdd; }
	void Reduce() {
		if (toAdd > 0)
			--toAdd;
		else if (chain.size() > hasTail)
			chain.erase(chain.begin());
	}
	void HasTail(bool answer) { hasTail = answer; }
	glm::vec2 GetTail(unsigned index = 1) {
		if (toAdd > 0)
			return chain[0];
		if (chain.size() <= index)
			return glm::vec2(0);
		return chain[index];
	}
	void InitSegments(unsigned amt) {
		toAdd = amt;
	}
	void AddSegment() {
		++toAdd;
	}
	void Update(glm::vec2 headPos) {
		chain.push_back(headPos);
		if (toAdd <= 0)		chain.erase(chain.begin());
		else				--toAdd;
	}
	void Draw() {
		for (int i(hasTail); i < chain.size(); ++i) {
			ObjLoader("models/Cube.obj").Draw(glm::mat4(
				{ 1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 1, 0 },
				{ chain[i].x, chain[i].y, 0.5f, 1 }
			));
		}
	}

private:
	std::vector<glm::vec2> chain = {};
	int toAdd = 0;
	bool hasTail = false;
};

