#pragma once
#include "cocos2d.h"
using namespace cocos2d;

class gameItem;

class gameScene: public cocos2d::Layer
{
	friend class gameItem;
public:
	gameScene(void);
	~gameScene(void);

	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// 触摸事件
	virtual bool onTouchBegan(Touch *touch, Event *unused_event);
	virtual void onTouchMoved(Touch *touch, Event *unused_event);
	virtual void onTouchEnded(Touch *touch, Event *unused_event);
	virtual void onTouchCancelled(Touch *touch, Event *unused_event);

	// 键盘事件 
	virtual void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

	// 根据游戏块相对位置，获取该块在游戏画面中的绝对坐标。
	cocos2d::Vec2 getItemPos(int row, int col);
	
	// 游戏块的移动处理
	bool moveLeft();
	bool moveRight();
	bool moveUp();
	bool moveDown();

	// 设置游戏过程中新增的 随机块
	void setRandomItem();
	//游戏结束判定
	bool gameoverJudge();	

	// 配置文件读写
	void readConfigFile();
	void saveConfigFile();
	
protected:
	// 游戏赢画面
	bool bGameWined;	/* 游戏是否已经赢过。(超过2048算赢但是可以继续玩下去)*/
	LayerColor *gameWinLayer;
	void showGameWinLayer(bool bshow = true);

private:
	cocos2d::Label *gameScoreText;	//
	cocos2d::Label *gameBestScoreText;
	int nBestScore;
	int nScore;	// 记录游戏总分数
	// 单步添加分数
	void AddGameScore(int step);
	bool bGameOver;	//游戏结束 变量(已经结束：true 否则: false)

	//gameItem *sp_item;
	cocos2d::Rect gameRect;		// 游戏区域 rect
	std::vector<gameItem *> itemList;	// 游戏块数组 
	std::list<gameItem *> emptyItemList;	/*空闲 item列表*/

	void gestureCallback(float dt);	// 滑动手势判定 <回调>
	int moveDistance;	// 移动距离判定阀值
	Vec2 pre_pos, cur_pos;	// 起始点位置， 当前点位置

	// (移动|合并) 整理 
	int item_sort(std::vector<gameItem *> item, int nsize = 4);

	// 游戏结束画面
	LayerColor *gameOverLayer;
	void showGameOverLayer(bool bshow = true);

	// 游戏暂停画面
	bool bGamePass;
	LayerColor *gamePassLayer;
	void showGamePassLayer(bool bshow = true);

	//重新开始游戏
	void ResetGameScene();
	// 恢复游戏
	void onResumeGame();
	// 游戏结束，退出程序
	void onEndGame();
	// about game
	void onAboutGame(){};

#if 0
	std::vector<gameItem *> itemList_temp;	// 游戏块数组--临时， 用于移动 
	void itemMoveEffects(int nsrc, int ndesc);	//游戏块移动效果
#endif
	// implement the "static create()" method manually
	CREATE_FUNC(gameScene);
};

/// 游戏块 类
class gameItem
{
public:
	gameItem(gameScene *handle);
	~gameItem(){};

	void setPosition(cocos2d::Vec2 pos);
	cocos2d::Vec2 getPosition() {return cocos2d::Vec2(pos_x, pos_y); };

	//设置节点数据，同时根据值设置颜色
	void setItemData(int itemData);	
	int getItemData() {return itemData; };

	// item节点伸缩特效 
	void itemScaleEffects();
	void runAction(ActionInterval *action);
	void showGameItem(bool bshow = true);

	// 获取随机数 <2|4> 
	int getRandNum();
private:
	gameScene *handle;  // 游戏场景 handle
	cocos2d::Sprite *spItem;
	cocos2d::Label *itemText;
	// 当前节点的相对位置 x|y = (1-4)
	int pos_x;
	int pos_y;
	int itemData;	// 当前节点数据 
};

