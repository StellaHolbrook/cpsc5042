#pragma once
class LocalContext
{
public:
	int getCurrentScore() { return score; }
	int updateScore() { return score++; }
private:
	int score = 0;
};