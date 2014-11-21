#include "gameScene.h"
#include "extensions/cocos-ext.h"
#include <string>
#include <sstream>

#include "external/tinyxml2/tinyxml2.h"
using namespace tinyxml2;

USING_NS_CC;
using namespace cocos2d::extension;

gameScene::gameScene(void)
{
	moveDistance = 25;	//初始设置触摸阀值。 
	nScore = 0;
	nBestScore = 0;
	
	bGameOver = false;
	gameOverLayer = nullptr;

	gamePassLayer = nullptr;
	bGamePass = false;

	bGameWined = false;
	gameWinLayer = nullptr;
}

gameScene::~gameScene(void)
{
}


Scene* gameScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();
	// 'layer' is an autorelease object
	auto layer = gameScene::create();
	// add layer as a child to scene
	scene->addChild(layer);
	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool gameScene::init()
{
	//////////////////////////////
	// 1. super init first
	if ( !Layer::init() )
	{
		return false;
	}

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// add "HelloWorld" splash screen"
	auto sprite = Sprite::create("images/background.png");
	// position the sprite on the center of the screen
	sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
	// add the sprite as a child to this layer
	this->addChild(sprite, 0);

	// cocos2d.. title
	//Label *bm_font = Label::createWithBMFont("fonts/boundsTestFont.fnt", "2048");
	//Label *bm_font = Label::create("2048", "Arial", 32);
	Label *bm_font = Label::createWithTTF("2048", "fonts/scriptbl.ttf", 42);
	bm_font->setPosition(visibleSize.width*0.22, visibleSize.height*0.86);
	this->addChild(bm_font);
	bm_font->setColor(Color3B(120,120,120));

	// 设置游戏区域 <触摸响应区域>
	gameRect = Rect(visibleSize.width*0.07,visibleSize.height*0.1, 
		visibleSize.width*0.86, visibleSize.height*0.48);

	//////////////////////////////////////////////////////////////////////////
	// 屏幕触摸时事件 
	this->setTouchEnabled(true);
	EventDispatcher *dispach = CCDirector::getInstance()->getEventDispatcher();

	this->setTouchMode(Touch::DispatchMode::ONE_BY_ONE);
	auto *listen = EventListenerTouchOneByOne::create();
	listen->onTouchBegan = CC_CALLBACK_2(gameScene::onTouchBegan, this);
	listen->onTouchMoved = CC_CALLBACK_2(gameScene::onTouchMoved, this);
	listen->onTouchEnded = CC_CALLBACK_2(gameScene::onTouchEnded, this);
	listen->onTouchCancelled = CC_CALLBACK_2(gameScene::onTouchCancelled, this);

	dispach->addEventListenerWithSceneGraphPriority(listen, this);

	//////////////////////////////////////////////////////////////////////////
	// 移动设备键盘按键响应事件
	this->setKeyboardEnabled(true);

	auto keyboardlisten = EventListenerKeyboard::create();
	keyboardlisten->onKeyPressed = CC_CALLBACK_2(gameScene::onKeyPressed, this);
	keyboardlisten->onKeyReleased = CC_CALLBACK_2(gameScene::onKeyReleased, this);
	
	dispach->addEventListenerWithSceneGraphPriority(keyboardlisten,this);

	//////////////////////////////////////////////////////////////////////////
	// 加载游戏分数记录 文本
	LayerColor *layer1 = LayerColor::create(Color4B(200,190,190,180));
	layer1->setPosition(Vec2(visibleSize.width*0.438, visibleSize.height*0.81));
	layer1->setContentSize(Size(visibleSize.width*0.22,visibleSize.height*0.06));
	addChild(layer1);
	LayerColor *layer2 = LayerColor::create(Color4B(200,190,190,180));
	layer2->setPosition(Vec2(visibleSize.width*0.69, visibleSize.height*0.81));
	layer2->setContentSize(Size(visibleSize.width*0.22,visibleSize.height*0.06));
	addChild(layer2);

	gameScoreText = Label::create("0", "fonts/mtcorsva.ttf", 24);
	gameScoreText->setPosition(Vec2(visibleSize.width*0.54, visibleSize.height*0.84));
	this->addChild(gameScoreText);
	AddGameScore(0);	// 初始化为0分

	gameBestScoreText = Label::create("0", "fonts/mtcorsva.ttf", 24);
	gameBestScoreText->setPosition(Vec2(visibleSize.width*0.80, visibleSize.height*0.84));
	this->addChild(gameBestScoreText);
	std::ostringstream ostrBest;
	ostrBest << nBestScore;
	gameBestScoreText->setString(ostrBest.str());
	//////////////////////////////////////////////////////////////////////////
	// 加载游戏精灵
	itemList.clear();
	for (int i = 0; i < 16; ++i)
	{
		gameItem *spItem = new gameItem(this);
		spItem->setPosition(getItemPos(i/4+1, i%4+1) );
		spItem->setItemData(0/*pow(2,i)*/);
		itemList.push_back(spItem);
		emptyItemList.push_back(spItem);
		
#if 0
		// 组件临时游戏块
		gameItem *spItem_t = new gameItem(this);
		spItem_t->setPosition(getItemPos(i/4+1, i%4+1) );
		spItem_t->setItemData(0);
		itemList_temp.push_back(spItem_t);
		spItem_t->showGameItem(false);
#endif
	}
	setRandomItem();
	setRandomItem();

	// 读取配置文件的历史记录
	readConfigFile();
	
	//////////////////////////////////////////////////////////////////////////
	// return pass 菜单选项
	MenuItemImage *menuitem = MenuItemImage::create("images/goto_not_selected.png","images/goto_selected.png",
		CC_CALLBACK_0(gameScene::showGamePassLayer, this, true));
	menuitem->setScale(0.8f);
	Menu *menu = Menu::create(menuitem, nullptr);
	menu->setPosition(Vec2(visibleSize.width*0.88, visibleSize.height*0.61));
	this->addChild(menu);

	return true;
}

// 配置文件读写
void gameScene::readConfigFile()
{
	std::string filePath = FileUtils::getInstance()->getWritablePath() + "config.xml";
	tinyxml2::XMLDocument *pDoc = new tinyxml2::XMLDocument();
	XMLError errorId = pDoc->LoadFile(filePath.c_str());

	if (errorId != 0) {
		//xml格式错误
		return;
	}

	XMLElement *rootElem = pDoc->RootElement();

	XMLElement *scoresElem = rootElem->FirstChildElement("scores");
	// 读取上局分数
	nScore = scoresElem->IntAttribute("score");
	gameScoreText->setString(String::createWithFormat("%d",nScore)->getCString());
	// 读取历史最高分数记录。
	nBestScore = scoresElem->IntAttribute("best");
	gameBestScoreText->setString(String::createWithFormat("%d", nBestScore)->getCString());

	XMLElement *itemsElem = rootElem->FirstChildElement("items");
	for (int i = 0; i < 16; ++i)
	{
		std::string item_name = String::createWithFormat("item%d",i+1)->getCString();
		
		// 当加载的块数据 >= 2048, 表示游戏已经赢了，但游戏继续玩下去。
		if (itemsElem->IntAttribute(item_name.c_str() ) >= 2048)
		{	// 预加载，bGameWined 赋值必须放在节点赋值前面，防止出现bug
			bGameWined = true;
			gameScoreText->setColor(Color3B(255,100,100));
		}

		itemList[i]->setItemData(itemsElem->IntAttribute(item_name.c_str() ) );
		
		// 当加载的块数据不为0，则将该块从空闲列表中移除
		if (itemsElem->IntAttribute(item_name.c_str() ) != 0)
		{
			emptyItemList.remove(itemList[i]);
		}
	}

	delete pDoc;
}

void gameScene::saveConfigFile()
{
	std::string filePath = FileUtils::getInstance()->getWritablePath() + "config.xml";
	CCLOG(filePath.c_str());
	tinyxml2::XMLDocument *pDoc = new tinyxml2::XMLDocument();

	//xml 声明（参数可选）
	XMLDeclaration *pDel = pDoc->NewDeclaration();

	pDoc->LinkEndChild(pDel);

	//添加<config>节点
	XMLElement *plistElement = pDoc->NewElement("config");
	pDoc->LinkEndChild(plistElement);

	//添加 <config>/<scores> 节点
	XMLElement *scoreElement = pDoc->NewElement("scores");
	plistElement->LinkEndChild(scoreElement);
	scoreElement->SetAttribute("score", String::createWithFormat("%d", nScore)->getCString());
	scoreElement->SetAttribute("best", String::createWithFormat("%d", nBestScore)->getCString());
	//添加 <config>/<items> 节点
	XMLElement *itemsElement = pDoc->NewElement("items");
	plistElement->LinkEndChild(itemsElement);
	for (int i = 0; i<16; i++) {
		itemsElement->SetAttribute(String::createWithFormat("item%d", i+1)->getCString(), 
			String::createWithFormat("%d", itemList[i]->getItemData())->getCString());
	}

	pDoc->SaveFile(filePath.c_str());
	delete pDoc;
}

bool gameScene::onTouchBegan(Touch *touch, Event *unused_event)
{
	Vec2 xpos = touch->getLocation();

	if (gameRect.containsPoint(xpos))
	{
		// 手势滑动 
		pre_pos = xpos;		//保存当前位置为起始位置
		cur_pos = xpos;
		scheduleOnce(schedule_selector(gameScene::gestureCallback), 0.18f);
	}
	return true;
}

void gameScene::onTouchMoved(Touch *touch, Event *unused_event)
{
	Vec2 xpos = touch->getLocation();
	cur_pos = xpos;		//随时获取当前移动点的位置
}

void gameScene::onTouchEnded(Touch *touch, Event *unused_event)
{
}

void gameScene::onTouchCancelled(Touch *touch, Event *unused_event)
{
}

// 键盘事件 
void gameScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
}

void gameScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	static int nClick = 0;
	saveConfigFile();
	if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		CCLOG("exit game. ");
		//SetConfigFile();
		if (bGamePass)
		{
			nClick = 1;
			showGamePassLayer(false);
			//showGameWinLayer(false);
		}
		else if(0 == nClick--)
		{
			Director::getInstance()->end();
		}
	}
	
//  	else if (keyCode == EventKeyboard::KeyCode::KEY_HOME)
//  	{
//  		SetConfigFile();
//  	}
	
}

void gameScene::gestureCallback( float dt )
{
	if (bGamePass)
	{
		return;
	}
	
	bool nRet = false;
	Size xsize = CCDirector::getInstance()->getVisibleSize();
	Vec2 sub = cur_pos - pre_pos;

	// 判断水平与竖直方向上 哪个偏移量大
	if (fabs(sub.x) > fabs(sub.y))	
	{	// 水平滑动
		if (sub.x > moveDistance)
		{
			CCLOG("right.");
			nRet = moveRight();
		}
		else if(sub.x < -moveDistance)
		{
			CCLOG("left.");
			nRet = moveLeft();
		}
	}
	else
	{	// 竖直滑动
		if (sub.y > moveDistance)
		{
			CCLOG("up.");
			nRet = moveUp();
		}
		else if(sub.y < -moveDistance)
		{
			CCLOG("down.");
			nRet = moveDown();
		}
	}

	// 滑动处理过后，自动生成新的随机块
	if ((fabs(sub.x) > moveDistance || fabs(sub.y) > moveDistance))
	{
		if (nRet)
		{
			setRandomItem();
		}

		// 每次滑动游戏区域，进行游戏结束判定。
		if (!bGameOver &&gameoverJudge())
		{
			bGameOver = true;
			showGameOverLayer();
		}
	}
}


Vec2 gameScene::getItemPos( int row, int col )
{
	Size xsize = CCDirector::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	Vec2 xpos;

	switch(row)
	{
	case 1:
		xpos.y = xsize.height*0.52 + origin.y;
		break;
	case 2:
		xpos.y = xsize.height*0.401 + origin.y;
		break;
	case 3:
		xpos.y = xsize.height*0.282 + origin.y;
		break;
	case 4:
		xpos.y = xsize.height*0.164 + origin.y;
		break;
	default:
		xpos.y = 0;
		break;
	}

	switch(col)
	{
	case 1:
		xpos.x = xsize.width*0.185 + origin.x;
		break;
	case 2:
		xpos.x = xsize.width*0.395 + origin.x;
		break;
	case 3:
		xpos.x = xsize.width*0.608 + origin.x;
		break;
	case 4:
		xpos.x = xsize.width*0.819 + origin.x;
		break;
	default:
		xpos.x = 0;
		break;
	}

	return xpos;
}

void gameScene::AddGameScore(int step)
{
	int score;
	// 积分算法
	score = (step == 0) ? 0 : (pow(2, step) + pow(step, 2));
	
	nScore += score;
	char *str = (char *)malloc(30);
	sprintf(str, "%d", nScore);
	gameScoreText->setString(str);

	if (nScore > nBestScore)
	{
		nBestScore = nScore;
		gameBestScoreText->setString(str);
	}
}

// (移动|合并) 整理 
int gameScene::item_sort(std::vector<gameItem *> item, int nsize)
{
	int nStepMove = 0, nStep=0;	// nStepMove:移步次数， nStep: 合并次数
	int n = 0;

	for(int i = 0; i < nsize; ++i)
	{
		if(item[i]->getItemData() == 0)	// item[i] != 0	  move item[++i] -> item[i]
		{
			n = i + 1;
			while (n < nsize)
			{
				// item[i] = 0  <- item[n] != 0  move item.
				if (item[n]->getItemData() != 0)
				{
					// move
					item[i]->setItemData(item[n]->getItemData());
					item[n]->setItemData(0);
					++nStepMove;
					//itemMoveEffects(n,i);
					// emptyItemList 列表处理
					emptyItemList.remove(item[i]);
					emptyItemList.push_back(item[n]);
					
					// after move, merge.  <such as: (0,2,0,2)>
					while(n < nsize)
					{
						if(item[n]->getItemData() != 0)	//item[++i] != 0   merge operation.
						{
							// item[i] = item[n] == 0  merge item.
							if(item[n]->getItemData() == item[i]->getItemData())
							{
								// merge
								item[i]->setItemData(item[i]->getItemData()*2);
								item[n]->setItemData(0);
								++nStep;
								//itemMoveEffects(n,i);
								item[i]->itemScaleEffects();
								// emptyItemList 列表处理
								emptyItemList.push_back(item[n]);
							}
							break;
						}
						++n;
					}
					break;
				}
				++n;
			}
		}
		else	// item[i] != 0	  merge item[++i] -> item[i]
		{
			n = i+1;
			while(n < nsize)
			{
				if(item[n]->getItemData() != 0)	//item[++i] != 0   merge operation.
				{
					// item[i] = item[n] == 0  merge item.
					if(item[n]->getItemData() == item[i]->getItemData())
					{
						// merge
						item[i]->setItemData(item[i]->getItemData()*2);
						item[n]->setItemData(0);
						++nStep;
						//itemMoveEffects(n,i);
						item[i]->itemScaleEffects();

						// emptyItemList 列表处理
						emptyItemList.push_back(item[n]);
					}
					break;
				}
				++n;
			}
		}
	}

	return (nStepMove>0)?nStep+1:nStep;
}

bool gameScene::moveLeft()
{
	int nStep = 0; // 块碰撞(移动|合并)的次数
	std::vector<gameItem *> item;

	for(int n = 0; n < 4; ++n)
	{
		item.clear();
		for (int i = 0; i < 4; ++i)
		{
			item.push_back(itemList[n*4+i]);
		}
		nStep += item_sort(item);
	}

	AddGameScore(nStep);
	return (nStep==0?false:true);
}

bool gameScene::moveRight()
{
	int nStep = 0; // 块碰撞(移动|合并)的次数
	std::vector<gameItem *> item;

	for(int n = 0; n < 4; ++n)
	{
		item.clear();
		for (int i = 0; i < 4; ++i)
		{
			item.push_back(itemList[(n+1)*4-i-1]);
		}
		nStep += item_sort(item);
	}

	AddGameScore(nStep);
	return (nStep==0?false:true);
}

bool gameScene::moveUp()
{
	int nStep = 0; // 块碰撞(移动|合并)的次数
	std::vector<gameItem *> item;

	for(int n = 0; n < 4; ++n)
	{
		item.clear();
		for (int i = 0; i < 4; ++i)
		{
			item.push_back(itemList[4*i+n]);
		}
		nStep += item_sort(item);
	}

	AddGameScore(nStep);
	return (nStep==0?false:true);
}

bool gameScene::moveDown()
{
	int nStep = 0; // 块碰撞(移动|合并)的次数
	std::vector<gameItem *> item;

	for(int n = 0; n < 4; ++n)
	{
		item.clear();
		for (int i = 0; i < 4; ++i)
		{
			item.push_back(itemList[4*(4-i-1)+n]);
		}
		nStep += item_sort(item);
	}

	AddGameScore(nStep);
	return (nStep==0?false:true);
}

// 设置一个随机数据块。并赋随机值。
void gameScene::setRandomItem()
{
	gameItem *sp;
	if (!emptyItemList.empty())
	{ 
		int xpos = cocos2d::random<int>(0,emptyItemList.size()-1);
		std::list<gameItem *>::iterator iter = emptyItemList.begin();
		for (int i = 0; iter != emptyItemList.end(); ++iter, ++i)
		{
			if (i == xpos)
			{
				 sp = *iter;
				 break;
			}
		}

		sp->setItemData(sp->getRandNum());
		emptyItemList.remove(sp);

		sp->itemScaleEffects();
	}
}

// 游戏结束 判定
// return false: 游戏尚未结束
// return true: 游戏判定结束
bool gameScene::gameoverJudge()
{
	bool bResult = true;
	int left, right, up, down;

	// 当item块列表中还存在空闲item时，游戏继续。
	if (!emptyItemList.empty())
	{
		return false;
	}
	
	for(int i=0; i < 16; ++i)
	{
		left = i-1;
		right = i+1;
		up = i-4;
		down = i+4;
		// i 位最左
		if (left > 0 && left < 16 && left%4 != 3)
		{
			if (itemList[left]->getItemData() == itemList[i]->getItemData())
			{
				bResult = false;
				break;
			}
		}
		// i 位最右
		if (right > 0 && right < 16 && right%4 != 0)
		{
			if (itemList[right]->getItemData() == itemList[i]->getItemData())
			{
				bResult = false;
				break;
			}
		}
		if (up > 0 && up < 16 )
		{
			if (itemList[up]->getItemData() == itemList[i]->getItemData())
			{
				bResult = false;
				break;
			}
		}
		if (down > 0 && down < 16 )
		{
			if (itemList[down]->getItemData() == itemList[i]->getItemData())
			{
				bResult = false;
				break;
			}
		}
	}

	return bResult;
}

void gameScene::ResetGameScene()
{
	// 重置分数
	bGameOver = false;
	showGameOverLayer(false);
	showGamePassLayer(false);
	showGameWinLayer(false);
	gameScoreText->setColor(Color3B(255, 255, 255));

	nScore = 0;
	gameScoreText->setString(cocos2d::String::createWithFormat("%d", nScore)->getCString() );
	emptyItemList.clear();
	for (int i = 0; i < 16; ++i)
	{
		itemList[i]->setItemData(0);
		emptyItemList.push_back(itemList[i]);
	}
	
	setRandomItem();
	setRandomItem();
}

void gameScene::showGameOverLayer( bool bshow )
{
	// if gameOverLayer is not created.
	if (gameOverLayer == nullptr)
	{
		//create gameOverLayer .
		gameOverLayer = LayerColor::create(Color4B(220,210,200,200));
		gameOverLayer->setPosition(gameRect.origin);
		gameOverLayer->setContentSize(gameRect.size);
		this->addChild(gameOverLayer, 60);

		Label *gameResult= Label::create("game over!", "fonts/scriptbl.ttf", 36);
		gameResult->setColor(Color3B(255,40,40));
		gameResult->setPosition(Vec2(gameRect.size.width/2, gameRect.size.height*0.75));
		gameOverLayer->addChild(gameResult);
		
		// restart game
		MenuItemLabel *restart_game = MenuItemLabel::create(Label::create("Restart", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::ResetGameScene, this));
		restart_game->setPosition(Vec2(gameRect.size.width/2, gameRect.size.height*0.5));
		// end game 
		MenuItemLabel *end_game = MenuItemLabel::create(Label::create("End game", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::onEndGame, this));
		end_game->setPosition(Vec2(gameRect.size.width/2, gameRect.size.height*0.35));

		Menu *menu = Menu::create(restart_game, end_game, nullptr);
		menu->setPosition(Vec2::ZERO);
		gameOverLayer->addChild(menu);
		menu->setColor(Color3B(255,255,255));
	}

	if (!bshow)
	{
		Hide *hide = Hide::create();
		gameOverLayer->runAction(hide);
	}
	else
	{
		Show *show = Show::create();
		gameOverLayer->runAction(show);
	}
}

void gameScene::showGamePassLayer( bool bshow )
{
	// gamePassLayer has't created. 
	if (gamePassLayer == nullptr)
	{
		Size size = Director::getInstance()->getVisibleSize();
		Vec2 origin = Director::getInstance()->getVisibleOrigin();

		gamePassLayer = LayerColor::create(Color4B(220,210,200, 220));
		gamePassLayer->setPosition(origin);
		gamePassLayer->setContentSize(size);
		this->addChild(gamePassLayer, 70);

		Label *gameTitle = Label::create("2048", "fonts/scriptbl.ttf", 36);
		//Label *gameTitle = Label::createWithBMFont("fonts/boundsTestFont.fnt", "2048");
		gameTitle->setColor(Color3B(120,120,120));
		gameTitle->setPosition(Vec2(size.width*0.5, size.height*0.8));
		gamePassLayer->addChild(gameTitle);
		gameTitle->setScale(1.5f);
		// resume game
		MenuItemLabel *resumegame = MenuItemLabel::create(Label::create("Resume", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::onResumeGame, this));
		resumegame->setPosition(Vec2(size.width/2, size.height*0.6));
		// restart game. 
		MenuItemLabel *restart = MenuItemLabel::create(Label::create("Restart", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::ResetGameScene, this));
		restart->setPosition(Vec2(size.width/2, size.height*0.5));
		//restart->setColor(Color3B(10,90,170));
		// exit game
		MenuItemLabel *exitgame = MenuItemLabel::create(Label::create("End game", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::onEndGame, this));
		exitgame->setPosition(Vec2(size.width/2, size.height*0.4));
		//exitgame->setColor(Color3B(10,90,170));
		// about game
		MenuItemLabel *aboutgame = MenuItemLabel::create(Label::create("about game", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::onAboutGame, this));
		aboutgame->setPosition(Vec2(size.width/2, size.height*0.3));

		Menu *menu = Menu::create(restart, exitgame, resumegame, aboutgame, nullptr);
		menu->setPosition(Vec2::ZERO);
		gamePassLayer->addChild(menu);
		menu->setColor(Color3B(255,255,255));

		Label *support= Label::create("support: jianyaoy@gmail.com", "Arial", 16);
		support->setColor(Color3B(255,255,255));
		support->setPosition(Vec2(size.width*0.5, size.height*0.03));
		gamePassLayer->addChild(support);
	}

	if (bshow)
	{
		bGamePass = true;
		Show *show = Show::create();
		gamePassLayer->runAction(show);
	}
	else
	{
		bGamePass = false;
		Hide *hide = Hide::create();
		gamePassLayer->runAction(hide);
	}
}

void gameScene::onEndGame()
{
	saveConfigFile();
	Director::getInstance()->end();
}

void gameScene::onResumeGame()
{
	showGamePassLayer(false);
}

void gameScene::showGameWinLayer( bool bshow /*= true*/ )
{
	// if gameOverLayer is not created.
	if (gameWinLayer == nullptr)
	{
		//create gamewinLayer .
		gameWinLayer = LayerColor::create(Color4B(220,210,200,200));
		gameWinLayer->setPosition(gameRect.origin);
		gameWinLayer->setContentSize(gameRect.size);
		this->addChild(gameWinLayer,65);

		Label *gameResult= Label::create("you win!", "fonts/scriptbl.ttf", 36);
		gameResult->setColor(Color3B(255,40,40));
		gameResult->setPosition(Vec2(gameRect.size.width/2, gameRect.size.height*0.85));
		gameWinLayer->addChild(gameResult);
		// continue
		MenuItemLabel *continue_game = MenuItemLabel::create(Label::create("Continue", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::showGameWinLayer, this, false));
		continue_game->setPosition(Vec2(gameRect.size.width/2, gameRect.size.height*0.6));
		// restart game
		MenuItemLabel *restart_game = MenuItemLabel::create(Label::create("Restart", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::ResetGameScene, this));
		restart_game->setPosition(Vec2(gameRect.size.width/2, gameRect.size.height*0.45));
		// end game 
		MenuItemLabel *end_game = MenuItemLabel::create(Label::create("End game", "fonts/scriptbl.ttf", 32),
			CC_CALLBACK_0(gameScene::onEndGame, this));
		end_game->setPosition(Vec2(gameRect.size.width/2, gameRect.size.height*0.3));

		Menu *menu = Menu::create(continue_game, restart_game, end_game, nullptr);
		menu->setPosition(Vec2::ZERO);
		gameWinLayer->addChild(menu);
		menu->setColor(Color3B(255,255,255));
	}

	if (!bshow)
	{
		Hide *hide = Hide::create();
		gameWinLayer->runAction(hide);
	}
	else
	{
		Show *show = Show::create();
		gameWinLayer->runAction(show);
	}
}

#if 0
void gameScene::itemMoveEffects(int nsrc, int ndesc)
{
	// 暂时不启用游戏块移动特效。 <看不出效果>
	return ;
	// 游戏移动特效。 
	Place *place = Place::create(itemList[nsrc]->getPosition());
	Hide *hide = Hide::create();
	Show *show = Show::create();
	MoveTo *move = MoveTo::create(0.2f, itemList[ndesc]->getPosition());

	itemList_temp[nsrc]->runAction(Sequence::create(place, show, move, hide, nullptr));
}

#endif




gameItem::gameItem( gameScene *handle )
{
	this->handle = handle; 

	spItem = Sprite::create("images/item.png");
	spItem->setPosition(handle->getItemPos(1,1));
	handle->addChild(spItem);
	itemText = Label::create("0", "Arial", 36);
	itemText->setPosition(handle->getItemPos(1,1));
	handle->addChild(itemText);
}

void gameItem::setPosition( cocos2d::Vec2 pos )
{
	spItem->setPosition(pos);
	itemText->setPosition(pos);

	pos_x = pos.x;
	pos_y = pos.y;
}

void gameItem::setItemData( int itemData )
{
	this->itemData = itemData;

	Color3B textColor;
	Color3B bgColor;
	char *str = (char *)malloc(20);
	sprintf(str, "%d", itemData);
	itemText->setString(str);

	if(strlen(str) == 3)
		itemText->setScaleX(0.85f);
	else if(strlen(str) == 4)
		itemText->setScaleX(0.7f);
	else if(strlen(str) == 5)
		itemText->setScaleX(0.55f);

	free(str);

	bgColor = Color3B(135,135,135);
	textColor = Color3B(255,255,255);

	switch (itemData)
	{
	case 0:
		bgColor = Color3B(220,210,200);
		itemText->setString("");
		break;
	case 2:
		bgColor = Color3B(240,230,220);
		textColor = Color3B(60,60,60);
		break;
	case 4:
		bgColor = Color3B(240,230,200);
		textColor = Color3B(60,60,60);
		break;
	case 8:
		bgColor = Color3B(240,170,120);
		break;
	case 16:
		bgColor = Color3B(240,180,120);
		break;
	case 32:
		bgColor = Color3B(240,140,90);
		break;
	case 64:
		bgColor = Color3B(240,120,90);
		break;
	case 128:
		bgColor = Color3B(240,90,60);
		break;
	case 256:
		bgColor = Color3B(230,80,40);
		break;
	case 512:
		bgColor = Color3B(240,60,40);
		break;
	case 1024:
		bgColor = Color3B(240,200,70);
		break;
	case 2048:
		bgColor = Color3B(230,230,0);
		break;
	case 4096:
		bgColor = Color3B(10,90,170);
		break;
	default:
		bgColor = Color3B(0,130,0);
		break;
	}

	spItem->setColor(bgColor);
	itemText->setColor(textColor);

	//
	if (itemData == 2048 && !handle->bGameWined)
	{
		handle->bGameWined = true;
		handle->gameScoreText->setColor(Color3B(255,100,100));
		handle->showGameWinLayer();
	}
	
}

int gameItem::getRandNum()
{
	return cocos2d::random<int>(1,10) > 8 ? 4 : 2;
}

// 添加游戏item新增动画特效
void gameItem::itemScaleEffects()
{
	//Blink *blink = Blink::create(1.0f, 1);	//闪烁
	spItem->setScale(0.85f);
	spItem->runAction(EaseBackOut::create(ScaleTo::create(1.0f,1.0f)));	//
}

void gameItem::runAction( ActionInterval *action )
{
	spItem->runAction(action);
	itemText->runAction(action);
}

void gameItem::showGameItem( bool bshow )
{
	if (bshow)
	{
		spItem->runAction(Show::create());
		itemText->runAction(Show::create());
	}
	else
	{
		spItem->runAction(Hide::create());
		itemText->runAction(Hide::create());
	}
}



