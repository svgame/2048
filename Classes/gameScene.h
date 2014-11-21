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

	// �����¼�
	virtual bool onTouchBegan(Touch *touch, Event *unused_event);
	virtual void onTouchMoved(Touch *touch, Event *unused_event);
	virtual void onTouchEnded(Touch *touch, Event *unused_event);
	virtual void onTouchCancelled(Touch *touch, Event *unused_event);

	// �����¼� 
	virtual void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

	// ������Ϸ�����λ�ã���ȡ�ÿ�����Ϸ�����еľ������ꡣ
	cocos2d::Vec2 getItemPos(int row, int col);
	
	// ��Ϸ����ƶ�����
	bool moveLeft();
	bool moveRight();
	bool moveUp();
	bool moveDown();

	// ������Ϸ������������ �����
	void setRandomItem();
	//��Ϸ�����ж�
	bool gameoverJudge();	

	// �����ļ���д
	void readConfigFile();
	void saveConfigFile();
	
protected:
	// ��ϷӮ����
	bool bGameWined;	/* ��Ϸ�Ƿ��Ѿ�Ӯ����(����2048��Ӯ���ǿ��Լ�������ȥ)*/
	LayerColor *gameWinLayer;
	void showGameWinLayer(bool bshow = true);

private:
	cocos2d::Label *gameScoreText;	//
	cocos2d::Label *gameBestScoreText;
	int nBestScore;
	int nScore;	// ��¼��Ϸ�ܷ���
	// ������ӷ���
	void AddGameScore(int step);
	bool bGameOver;	//��Ϸ���� ����(�Ѿ�������true ����: false)

	//gameItem *sp_item;
	cocos2d::Rect gameRect;		// ��Ϸ���� rect
	std::vector<gameItem *> itemList;	// ��Ϸ������ 
	std::list<gameItem *> emptyItemList;	/*���� item�б�*/

	void gestureCallback(float dt);	// ���������ж� <�ص�>
	int moveDistance;	// �ƶ������ж���ֵ
	Vec2 pre_pos, cur_pos;	// ��ʼ��λ�ã� ��ǰ��λ��

	// (�ƶ�|�ϲ�) ���� 
	int item_sort(std::vector<gameItem *> item, int nsize = 4);

	// ��Ϸ��������
	LayerColor *gameOverLayer;
	void showGameOverLayer(bool bshow = true);

	// ��Ϸ��ͣ����
	bool bGamePass;
	LayerColor *gamePassLayer;
	void showGamePassLayer(bool bshow = true);

	//���¿�ʼ��Ϸ
	void ResetGameScene();
	// �ָ���Ϸ
	void onResumeGame();
	// ��Ϸ�������˳�����
	void onEndGame();
	// about game
	void onAboutGame(){};

#if 0
	std::vector<gameItem *> itemList_temp;	// ��Ϸ������--��ʱ�� �����ƶ� 
	void itemMoveEffects(int nsrc, int ndesc);	//��Ϸ���ƶ�Ч��
#endif
	// implement the "static create()" method manually
	CREATE_FUNC(gameScene);
};

/// ��Ϸ�� ��
class gameItem
{
public:
	gameItem(gameScene *handle);
	~gameItem(){};

	void setPosition(cocos2d::Vec2 pos);
	cocos2d::Vec2 getPosition() {return cocos2d::Vec2(pos_x, pos_y); };

	//���ýڵ����ݣ�ͬʱ����ֵ������ɫ
	void setItemData(int itemData);	
	int getItemData() {return itemData; };

	// item�ڵ�������Ч 
	void itemScaleEffects();
	void runAction(ActionInterval *action);
	void showGameItem(bool bshow = true);

	// ��ȡ����� <2|4> 
	int getRandNum();
private:
	gameScene *handle;  // ��Ϸ���� handle
	cocos2d::Sprite *spItem;
	cocos2d::Label *itemText;
	// ��ǰ�ڵ�����λ�� x|y = (1-4)
	int pos_x;
	int pos_y;
	int itemData;	// ��ǰ�ڵ����� 
};

