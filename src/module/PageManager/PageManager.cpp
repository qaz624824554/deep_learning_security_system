#include "PageManager.hpp"
#include "UI.hpp"

void PageManager::init(Camera & camera, FaceRknnPool & face_rknn_pool, ImageProcess & image_process)
{
    // 创建所有页面
    pages_[PageType::ACCESS_CONTROL_PAGE] = std::make_unique<AccessControlPage>(camera, face_rknn_pool, image_process);
    pages_[PageType::MAIN_PAGE] = std::make_unique<MainPage>();
    
    // 设置初始页面
    current_page_ = pages_[PageType::ACCESS_CONTROL_PAGE].get();
}

void PageManager::switchToPage(PageType pageType)
{
    auto it = pages_.find(pageType);
    if(it != pages_.end()) {
        if(current_page_) {
            current_page_->hide();
        }
        current_page_ = it->second.get();
        current_page_->show();
    }
}