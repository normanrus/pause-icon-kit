#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GJGarageLayer.hpp>

using namespace geode::prelude;

bool g_fromPauseMenu = false;
PauseLayer* g_savedPauseLayer = nullptr;

class MyAlertDelegate : public FLAlertLayerProtocol {
public:
    static MyAlertDelegate* get() {
        static MyAlertDelegate instance;
        return &instance;
    }

    void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override {
        // Закрываем сцену гаража
        CCDirector::get()->popScene();

        if (auto playLayer = PlayLayer::get()) {
            auto gm = GameManager::get();
            if (playLayer->m_player1) {
                playLayer->m_player1->updatePlayerFrame(gm->activeIconForType(gm->m_playerIconType));
            }
            if (playLayer->m_player2) {
                playLayer->m_player2->updatePlayerFrame(gm->activeIconForType(gm->m_playerIconType));
            }

            // ИСПРАВЛЕННЫЙ ФИКС: Вызываем оригинальный метод onResume, который точно есть в PauseLayer.hpp.
            // Он закроет меню паузы и вернет управление в игру.
            if (g_savedPauseLayer) {
                g_savedPauseLayer->onResume(nullptr);
            }
        }
        
        g_savedPauseLayer = nullptr;
    }
};

class $modify(MyPauseLayer, PauseLayer) {
    
    void customSetup() {
        PauseLayer::customSetup();

        auto rightMenu = typeinfo_cast<CCMenu*>(this->getChildByID("right-button-menu"));
        if (!rightMenu) return;

        auto garageSprite = CCSprite::createWithSpriteFrameName("GJ_garageBtn_001.png");
        garageSprite->setScale(0.8f);

        auto garageBtn = CCMenuItemSpriteExtra::create(
            garageSprite,
            this,
            menu_selector(MyPauseLayer::onGarageClick)
        );

        garageBtn->setID("garage-button"_spr);
        rightMenu->addChild(garageBtn);
        rightMenu->updateLayout();
    }

    void onGarageClick(CCObject* sender) {
        g_fromPauseMenu = true;
        g_savedPauseLayer = this;

        auto garageScene = GJGarageLayer::scene();
        auto transition = CCTransitionFade::create(0.5f, garageScene);
        CCDirector::get()->pushScene(transition);
    }
};

class $modify(MyGarageLayer, GJGarageLayer) {
    
    void showExperimentalWarning() {
        if (g_fromPauseMenu) {
            g_fromPauseMenu = false;

            auto alert = FLAlertLayer::create(
                MyAlertDelegate::get(),
                "EXPERIMENTAL MODE",
                "You are leaving the garage.\n\n"
                "<cr>Warning:</c> The level will resume immediately!", 
                "LET'S GO!",
                nullptr,
                300.0f
            );
            alert->show();
        } else {
            GJGarageLayer::onBack(nullptr);
        }
    }

    void onBack(CCObject* sender) {
        showExperimentalWarning();
    }

    void keyBackClicked() {
        showExperimentalWarning();
    }
};