#include "Inspector.h"
#include "ImGuiPresenter.h"
#include "axmol.h"

#if __has_include(<cxxabi.h>)
#    define AX_HAS_CXXABI 1
#    include <cxxabi.h>
#endif

#if !defined(FMT_HEADER_ONLY)
#    define FMT_HEADER_ONLY
#endif

#include "fmt/format.h"
#include <memory>
#include <imgui/misc/cpp/imgui_stdlib.h>

NS_AX_EXT_BEGIN

static Inspector* _instance = nullptr;

Inspector* Inspector::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new Inspector();
        _instance->init();
    }
    return _instance;
}

void Inspector::destroyInstance()
{
    if (_instance)
    {
        _instance->close();
        _instance->cleanup();
        delete _instance;
        _instance = nullptr;
    }
}

void Inspector::init()
{
}

void Inspector::cleanup()
{
}

#if AX_TARGET_PLATFORM == AX_PLATFORM_WIN32

std::string Inspector::demangle(const char* name)
{
    // works because msvc's typeid().name() returns undecorated name
    // typeid(Node).name() == "class ax::Node"
    // the + 6 gets rid of the class prefix
    // "class ax::Node" + 6 == "ax::Node"
    return std::string(name + 6);
}

#elif AX_HAS_CXXABI

std::string Inspector::demangle(const char* mangled_name)
{
    int status = -4;
    std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status), std::free};
    return (status == 0) ? res.get() : mangled_name;
}

#else

std::string Inspector::demangle(const char* name)
{
    return std::string(name);
}

#endif

std::string Inspector::getNodeName(Node* node)
{
    return Inspector::demangle(typeid(*node).name());
}

void Inspector::drawTreeRecusrive(Node* node, int index)
{
    std::string str = fmt::format("[{}] {}", index, Inspector::getNodeName(node));

    if (node->getTag() != -1)
    {
        str += fmt::format(" ({})", node->getTag());
    }

    const auto childrenCount = node->getChildrenCount();
    if (childrenCount != 0)
    {
        str += fmt::format(" {{{}}}", childrenCount);
    }

    auto flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
    if (_selected_node == node)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (node->getChildrenCount() == 0)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool is_open = ImGui::TreeNodeEx(node, flags, "%s", str.c_str());

    if (ImGui::IsItemClicked())
    {
        if (node == _selected_node && ImGui::GetIO().KeyAlt)
        {
            _selected_node = nullptr;
        }
        else
        {
            _selected_node = node;
        }
    }

    const auto children_count = node->getChildrenCount();

    if (is_open)
    {
        const auto &children = node->getChildren();
        for (int i = 0; auto* child : children)
        {
            if(!child)
            {
                continue;
            }

            drawTreeRecusrive(child, i);
            i++;
        }
        ImGui::TreePop();
    }
}

void Inspector::drawProperties()
{
    if (_selected_node == nullptr)
    {
        ImGui::Text("Select a node to edit its properties :-)");
        return;
    }

    if (ImGui::Button("Delete"))
    {
        _selected_node->removeFromParentAndCleanup(true);
        _selected_node = nullptr;
        return;
    }

    //ImGui::SameLine();
    //
    //if (ImGui::Button("Add Child"))
    //{
    //    ImGui::OpenPopup("Add Child");
    //}
    //
    //if (ImGui::BeginPopupModal("Add Child"))
    //{
    //    static int item = 0;
    //    ImGui::Combo("Node", &item, "Node\0LabelBMFont\0LabelTTF\0Sprite\0MenuItemSpriteExtra\0");
    //
    //    static int tag = -1;
    //    ImGui::InputInt("Tag", &tag);
    //
    //    static char text[256]{0};
    //    if (item == 1)
    //    {
    //        static char labelFont[256]{0};
    //        ImGui::InputText("Text", text, 256);
    //        ImGui::InputText("Font", labelFont, 256);
    //    }
    //    static int fontSize = 20;
    //    if (item == 2)
    //    {
    //        ImGui::InputText("Text", text, 256);
    //        ImGui::InputInt("Font Size", &fontSize);
    //    }
    //    static bool frame = false;
    //    if (item == 3 || item == 4)
    //    {
    //        ImGui::InputText("Texture", text, 256);
    //        ImGui::Checkbox("Frame", &frame);
    //    }
    //
    //    ImGui::Separator();
    //    ImGui::SameLine();
    //    if (ImGui::Button("Cancel"))
    //    {
    //        ImGui::CloseCurrentPopup();
    //    }
    //    ImGui::EndPopup();
    //}

    ImGui::Text("Addr: %p", _selected_node);
    if (auto userData = _selected_node->getUserData(); userData)
    {
        ImGui::SameLine();
        ImGui::Text("User data: %p", userData);
    }

    auto pos = _selected_node->getPosition();
    float _pos[2] = {pos.x, pos.y};
    ImGui::DragFloat2("Position", _pos);
    _selected_node->setPosition({_pos[0], _pos[1]});

        // need to use getScaleX() because of assert
    float _scale[3] = {_selected_node->getScaleX(), _selected_node->getScaleX(), _selected_node->getScaleY()};
    ImGui::DragFloat3("Scale", _scale, 0.025f);

    if (auto scale_0 = _selected_node->getScaleX(); scale_0 != _scale[0])
    {
        _selected_node->setScale(_scale[0]);
    }
    else
    {
        _selected_node->setScaleX(_scale[1]);
        _selected_node->setScaleY(_scale[2]);
    }

    float rotation[3] = {_selected_node->getRotationSkewX(), _selected_node->getRotationSkewX(),
                         _selected_node->getRotationSkewY()};
    if (ImGui::DragFloat3("Rotation", rotation, 1.0f))
    {
        if (_selected_node->getRotationSkewX() != rotation[0])
        {
            _selected_node->setRotation(rotation[0]);
        }
        else
        {
            _selected_node->setRotationSkewX(rotation[1]);
            _selected_node->setRotationSkewY(rotation[2]);
        }
    }

    auto contect = _selected_node->getContentSize();
    float _cont[2] = {contect.x, contect.y};
    ImGui::DragFloat2("Content Size", _cont);
    _selected_node->setContentSize({_cont[0], _cont[1]});

    auto anchor = _selected_node->getAnchorPoint();
    float _anch[2] = {anchor.x, anchor.y};
    ImGui::DragFloat2("Anchor Point", _anch);
    _selected_node->setAnchorPoint({_anch[0], _anch[1]});

    int zOrder = _selected_node->getLocalZOrder();
    ImGui::InputInt("Z", &zOrder);
    if (_selected_node->getLocalZOrder() != zOrder)
    {
        _selected_node->setLocalZOrder(zOrder);
    }

    auto visible = _selected_node->isVisible();
    ImGui::Checkbox("Visible", &visible);
    if (visible != _selected_node->isVisible())
    {
        _selected_node->setVisible(visible);
    }

    if (auto sprite = dynamic_cast<ax::Sprite*>(_selected_node))
    {
        ImGui::SameLine();
        bool flipx = sprite->isFlippedX();
        bool flipy = sprite->isFlippedY();
        ImGui::Checkbox("FlipX", &flipx);
        ImGui::SameLine();
        ImGui::Checkbox("FlipY", &flipy);
        sprite->setFlippedX(flipx);
        sprite->setFlippedY(flipy);
    }


    auto color = _selected_node->getColor();
    float _color[4] = {color.r / 255.f, color.g / 255.f, color.b / 255.f, _selected_node->getOpacity() / 255.f};
    ImGui::ColorEdit4("Color", _color);
    _selected_node->setColor({static_cast<GLubyte>(_color[0] * 255), static_cast<GLubyte>(_color[1] * 255),
                              static_cast<GLubyte>(_color[2] * 255)});
    _selected_node->setOpacity(static_cast<int>(_color[3] * 255.f));


    if (auto label_node = dynamic_cast<LabelProtocol*>(_selected_node); label_node)
    {
        std::string_view label = label_node->getString();
        std::string label_str(label.begin(), label.end());

        if (ImGui::InputTextMultiline("Text", &label_str, {0, 50}))
        {
            label_node->setString(label_str);
        }
    }

    if (auto sprite = dynamic_cast<ax::Sprite*>(_selected_node))
    {
        auto texture = sprite->getTexture();
        ImGui::TextWrapped("Texture: %s", texture->getPath().c_str());
    }
}

void Inspector::openForScene(Scene* target)
{
    _target = target;

    auto* presenter = ImGuiPresenter::getInstance();
    presenter->addFont(FileUtils::getInstance()->fullPathForFilename("fonts/arial.ttf"));
    presenter->enableDPIScale();
    presenter->addRenderLoop("#insp", AX_CALLBACK_0(Inspector::mainLoop , this), target);
}

void Inspector::close()
{
    _selected_node = nullptr;
    _target = nullptr;

    auto presenter = ImGuiPresenter::getInstance();
    presenter->removeRenderLoop("#insp");
    presenter->clearFonts();
}

void Inspector::mainLoop()
{
    if(!_target)
    {
        close();
        return;
    }

    if (ImGui::Begin("Inspector"))
    {
        const auto avail = ImGui::GetContentRegionAvail();
        if (ImGui::BeginChild("node.explorer.tree", ImVec2(avail.x * 0.5f, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            drawTreeRecusrive(_target);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("node.explorer.options"))
        {
            drawProperties();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

NS_AX_EXT_END
