/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2012-2018 DragonBones team and other contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef DRAGONBONES_CC_FACTORY_H
#define DRAGONBONES_CC_FACTORY_H

#include "dragonBones/DragonBonesHeaders.h"
#include "cocos2d.h"
#include "CCArmatureDisplay.h"

DRAGONBONES_NAMESPACE_BEGIN
/**
 * The Cocos2d factory.
 * @version DragonBones 3.0
 * @language en_US
 */
/**
 * Cocos2d ������
 * @version DragonBones 3.0
 * @language zh_CN
 */
class CCFactory : public BaseFactory
{
    DRAGONBONES_DISALLOW_COPY_AND_ASSIGN(CCFactory)

private:
    static DragonBones* _dragonBonesInstance;
    static CCFactory* _factory;

public:
    /**
     * A global factory instance that can be used directly.
     * @version DragonBones 4.7
     * @language en_US
     */
    /**
     * һ������ֱ��ʹ�õ�ȫ�ֹ���ʵ����
     * @version DragonBones 4.7
     * @language zh_CN
     */
    static CCFactory* getFactory()
    {
        if (CCFactory::_factory == nullptr) 
        {
            CCFactory::_factory = new CCFactory();
        }

        return CCFactory::_factory;
    }

protected:
    std::string _prevPath;

public:
    /**
     * @inheritDoc
     */
    CCFactory() :
        _prevPath()
    {
        if (_dragonBonesInstance == nullptr)
        {
            const auto eventManager = CCArmatureDisplay::create();
            eventManager->retain();

            _dragonBonesInstance = new DragonBones(eventManager);
            _dragonBonesInstance->yDown = false;

            cocos2d::Director::getInstance()->getScheduler()->schedule(
                [&](float passedTime)
                {
                    _dragonBonesInstance->advanceTime(passedTime);
                },
                this, 0.0f, false, "dragonBonesClock"
            );
        }

        _dragonBones = _dragonBonesInstance;
    }
    virtual ~CCFactory() 
    {
        clear();
    }

protected:
    virtual TextureAtlasData* _buildTextureAtlasData(TextureAtlasData* textureAtlasData, void* textureAtlas) const override;
    virtual Armature* _buildArmature(const BuildArmaturePackage& dataPackage) const override;
    virtual Slot* _buildSlot(const BuildArmaturePackage& dataPackage, const SlotData* slotData, Armature* armature) const override;

public:
    virtual DragonBonesData* loadDragonBonesData(const std::string& filePath, const std::string& name = "", float scale = 1.0f);
    /**
     * - Load and parse a texture atlas data and texture from the local and cache them to the factory.
     * @param  filePath - The file path of texture atlas data.
     * @param name - Specify a cache name for the instance so that the instance can be obtained through this name. (If not set, use the instance name instead)
     * @param scale - Specify a scaling value for the map set. (Not scaled by default)
     * @returns The TextureAtlasData instance.
     * @version DragonBones 4.5
     * @example
     * <pre>
     *     factory.loadTextureAtlasData("hero_tex.json");
     * </pre>
     * @language en_US
     */
    /**
     * - �ӱ��ؼ��ز�����һ����ͼ�����ݺ���ͼ�����浽�����С�
     * @param filePath - ��ͼ�������ļ�·����
     * @param name - Ϊ��ʵ��ָ��һ���������ƣ��Ա����ͨ�������ƻ�ȡ��ʵ���� �����δ���ã���ʹ�ø�ʵ���е����ƣ�
     * @param scale - Ϊ��ͼ��ָ��һ������ֵ�� ��Ĭ�ϲ����ţ�
     * @returns TextureAtlasData ʵ����
     * @version DragonBones 4.5
     * @example
     * <pre>
     *     factory.loadTextureAtlasData("hero_tex.json");
     * </pre>
     * @language zh_CN
     */
    virtual TextureAtlasData* loadTextureAtlasData(const std::string& filePath, const std::string& name = "", float scale = 1.0f);
    /**
     * - Create a armature from cached DragonBonesData instances and TextureAtlasData instances, then use the {@link #clock} to update it.
     * The difference is that the armature created by {@link #buildArmature} is not WorldClock instance update.
     * @param armatureName - The armature data name.
     * @param dragonBonesName - The cached name of the DragonBonesData instance. (If not set, all DragonBonesData instances are retrieved, and when multiple DragonBonesData instances contain a the same name armature data, it may not be possible to accurately create a specific armature)
     * @param skinName - The skin name, you can set a different ArmatureData name to share it's skin data. (If not set, use the default skin data)
     * @returns The armature display container.
     * @version DragonBones 4.5
     * @example
     * <pre>
     *     let armatureDisplay = factory.buildArmatureDisplay("armatureName", "dragonBonesName");
     * </pre>
     * @language en_US
     */
    /**
     * - ͨ������� DragonBonesData ʵ���� TextureAtlasData ʵ������һ���Ǽܣ����� {@link #clock} ���¸ùǼܡ�
     * ���������� {@link #buildArmature} �����ĹǼ�û�� WorldClock ʵ��������
     * @param armatureName - �Ǽ��������ơ�
     * @param dragonBonesName - DragonBonesData ʵ���Ļ������ơ� �����δ���ã����������е� DragonBonesData ʵ��������� DragonBonesData ʵ���а���ͬ���ĹǼ�����ʱ�������޷�׼ȷ�Ĵ������ض��ĹǼܣ�
     * @param skinName - Ƥ�����ƣ���������һ�������Ǽ�����������������Ƥ�����ݡ� �����δ���ã���ʹ��Ĭ�ϵ�Ƥ�����ݣ�
     * @returns �Ǽܵ���ʾ������
     * @version DragonBones 4.5
     * @example
     * <pre>
     *     let armatureDisplay = factory.buildArmatureDisplay("armatureName", "dragonBonesName");
     * </pre>
     * @language zh_CN
     */
    virtual CCArmatureDisplay* buildArmatureDisplay(const std::string& armatureName, const std::string& dragonBonesName = "", const std::string& skinName = "", const std::string& textureAtlasName = "") const;
    /**
     * - Create the display object with the specified texture.
     * @param textureName - The texture data name.
     * @param textureAtlasName - The texture atlas data name. (Of not set, all texture atlas data will be searched)
     * @version DragonBones 3.0
     * @language en_US
     */
    /**
     * - ��������ָ����ͼ����ʾ����
     * @param textureName - ��ͼ�������ơ�
     * @param textureAtlasName - ��ͼ���������ơ� �����δ���ã����������е���ͼ�����ݣ�
     * @version DragonBones 3.0
     * @language zh_CN
     */
    virtual cocos2d::Sprite* getTextureDisplay(const std::string& textureName, const std::string& dragonBonesName = "") const;
    /**
     * - A global sound event manager.
     * Sound events can be listened to uniformly from the manager.
     * @version DragonBones 4.5
     * @language en_US
     */
    /**
     * - ȫ�������¼���������
     * �����¼����ԴӸù�����ͳһ������
     * @version DragonBones 4.5
     * @language zh_CN
     */
    virtual CCArmatureDisplay* getSoundEventManager() const
    {
        return dynamic_cast<CCArmatureDisplay*>(static_cast<IArmatureProxy*>(_dragonBones->getEventManager()));
    }

    /**
     * Deprecated, please refer to {@link #clock}.
     * @deprecated
     * @language en_US
     */
    /**
     * �ѷ�������ο� {@link #clock}��
     * @deprecated
     * @language zh_CN
     */
    static WorldClock* getClock()
    {
        return _dragonBonesInstance->getClock();
    }
};

DRAGONBONES_NAMESPACE_END
#endif // DRAGONBONES_CC_FACTORY_H
