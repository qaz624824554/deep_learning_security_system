#pragma once

#include "Camera.hpp"
#include "RknnPool.hpp"
#include "ImageProcess.hpp"

class BasePage {
  public:
    virtual ~BasePage() = default;
    virtual void show() = 0;
    virtual void hide() = 0;
};

class PageManager {
  public:
    // 页面类型枚举
    enum class PageType {
        ACCESS_CONTROL_PAGE,
        MAIN_PAGE,
        SECURITY_CAMERA_PAGE,
    };

    static PageManager & getInstance()
    {
        static PageManager instance;
        return instance;
    }

    // 初始化所有页面
    void init(Camera & camera, FaceRknnPool & face_rknn_pool, ImageProcess & image_process);

    // 切换到指定页面
    void switchToPage(PageType pageType);

    // 获取当前页面
    BasePage * getCurrentPage() const
    {
        return current_page_;
    }

  private:
    PageManager()  = default;
    ~PageManager() = default;

    // 禁用拷贝和移动
    PageManager(const PageManager &)             = delete;
    PageManager & operator=(const PageManager &) = delete;

    std::unordered_map<PageType, std::unique_ptr<BasePage>> pages_;
    BasePage * current_page_{nullptr};
};