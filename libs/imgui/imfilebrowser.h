﻿#pragma once

#include <iostream>
#include <array>
#include <functional>
#include <memory>
#include <string>

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include)
  #if __has_include(<filesystem>)
  #define GHC_USE_STD_FS
  #include <filesystem>
    namespace fs = std::filesystem;
  #endif
#endif
#ifndef GHC_USE_STD_FS
  #include <ghc/filesystem.hpp>
  namespace fs = ghc::filesystem;
#endif

#ifndef IMGUI_VERSION
#   error "include imgui.h before this header"
#endif

using ImGuiFileBrowserFlags = int;

enum ImGuiFileBrowserFlags_
{
    ImGuiFileBrowserFlags_SelectDirectory    = 1 << 0, // select directory instead of regular file
    ImGuiFileBrowserFlags_EnterNewFilename   = 1 << 1, // allow user to enter new filename when selecting regular file
    ImGuiFileBrowserFlags_NoModal            = 1 << 2, // file browsing window is modal by default. specify this to use a popup window
    ImGuiFileBrowserFlags_NoTitleBar         = 1 << 3, // hide window title bar
    ImGuiFileBrowserFlags_NoStatusBar        = 1 << 4, // hide status bar at the bottom of browsing window
    ImGuiFileBrowserFlags_CloseOnEsc         = 1 << 5, // close file browser when pressing 'ESC'
    ImGuiFileBrowserFlags_CreateNewDir       = 1 << 6, // allow user to create new directory
};

namespace ImGui
{
    class FileBrowser
    {
    public:

        // pwd is set to current working directory by default
        explicit FileBrowser(ImGuiFileBrowserFlags flags = 0);

        FileBrowser(const FileBrowser &copyFrom);

        FileBrowser &operator=(const FileBrowser &copyFrom);

        // set the window title text
        void SetTitle(std::string title);

        // open the browsing window
        void Open();

        // close the browsing window
        void Close();

        // the browsing window is opened or not
        bool IsOpened() const noexcept;

        // display the browsing window if opened
        void Display();

        // returns true when there is a selected filename and the "ok" button was clicked
        bool HasSelected() const noexcept;

        // set current browsing directory
        bool SetPwd(const fs::path &pwd = fs::current_path());

        // returns selected filename. make sense only when HasSelected returns true
        fs::path GetSelected() const;

        // set selected filename to empty
        void ClearSelected();

        void SetTypeFilters(const std::vector<const char*> &typeFilters);

    private:

        class ScopeGuard
        {
            std::function<void()> func_;

        public:

            template<typename T>
            explicit ScopeGuard(T func) : func_(std::move(func)) { }
            ~ScopeGuard() { func_(); }
        };

        void SetPwdUncatched(const fs::path &pwd);

#ifdef _WIN32
        static std::uint32_t GetDrivesBitMask();
#endif

        ImGuiFileBrowserFlags flags_;

        std::string title_;
        std::string openLabel_;

        bool openFlag_;
        bool closeFlag_;
        bool isOpened_;
        bool ok_;

        std::string statusStr_;

        std::vector<const char*> typeFilters_;
        int typeFilterIndex_;

        fs::path pwd_;
        std::string selectedFilename_;

        struct FileRecord
        {
            bool isDir = false;
            std::string name;
            std::string showName;
            std::string extension;
        };
        std::vector<FileRecord> fileRecords_;

        // IMPROVE: overflow when selectedFilename_.length() > inputNameBuf_.size() - 1
        static constexpr size_t INPUT_NAME_BUF_SIZE = 512;
        std::unique_ptr<std::array<char, INPUT_NAME_BUF_SIZE>> inputNameBuf_;

        std::string openNewDirLabel_;
        std::unique_ptr<std::array<char, INPUT_NAME_BUF_SIZE>> newDirNameBuf_;

#ifdef _WIN32
        uint32_t drives_;
#endif
    };
} // namespace ImGui

inline ImGui::FileBrowser::FileBrowser(ImGuiFileBrowserFlags flags)
    : flags_(flags),
      openFlag_(false), closeFlag_(false), isOpened_(false), ok_(false),
      inputNameBuf_(std::make_unique<std::array<char, INPUT_NAME_BUF_SIZE>>())
{
    if(flags_ & ImGuiFileBrowserFlags_CreateNewDir)
        newDirNameBuf_ = std::make_unique<std::array<char, INPUT_NAME_BUF_SIZE>>();

    inputNameBuf_->at(0) = '\0';
    SetTitle("file browser");
    SetPwd(fs::current_path());

    typeFilters_.clear();
    typeFilterIndex_ = 0;

#ifdef _WIN32
    drives_ = GetDrivesBitMask();
#endif
}

inline ImGui::FileBrowser::FileBrowser(const FileBrowser &copyFrom)
    : FileBrowser()
{
    *this = copyFrom;
}

inline ImGui::FileBrowser &ImGui::FileBrowser::operator=(const FileBrowser &copyFrom)
{
    flags_ = copyFrom.flags_;
    SetTitle(copyFrom.title_);

    openFlag_  = copyFrom.openFlag_;
    closeFlag_ = copyFrom.closeFlag_;
    isOpened_  = copyFrom.isOpened_;
    ok_        = copyFrom.ok_;

    statusStr_ = "";
    pwd_ = copyFrom.pwd_;
    selectedFilename_ = copyFrom.selectedFilename_;

    fileRecords_ = copyFrom.fileRecords_;

    *inputNameBuf_ = *copyFrom.inputNameBuf_;

    if(flags_ & ImGuiFileBrowserFlags_CreateNewDir)
    {
        newDirNameBuf_ = std::make_unique<std::array<char, INPUT_NAME_BUF_SIZE>>();
        *newDirNameBuf_ = *copyFrom.newDirNameBuf_;
    }

    return *this;
}

inline void ImGui::FileBrowser::SetTitle(std::string title)
{
    title_ = std::move(title);
    openLabel_ = title_ + "##filebrowser_" + std::to_string(reinterpret_cast<size_t>(this));
    openNewDirLabel_ = "new dir##new_dir_" + std::to_string(reinterpret_cast<size_t>(this));
}

inline void ImGui::FileBrowser::Open()
{
    ClearSelected();
    statusStr_ = std::string();
    openFlag_ = true;
    closeFlag_ = false;
    #ifdef _WIN32
    drives_ = GetDrivesBitMask();
    #endif
}

inline void ImGui::FileBrowser::Close()
{
    ClearSelected();
    statusStr_ = std::string();
    closeFlag_ = true;
    openFlag_ = false;
}

inline bool ImGui::FileBrowser::IsOpened() const noexcept
{
    return isOpened_;
}

inline void ImGui::FileBrowser::Display()
{
    PushID(this);
    ScopeGuard exitThis([this] { openFlag_ = false; closeFlag_ = false; PopID(); });

    if(openFlag_)
        OpenPopup(openLabel_.c_str());
    isOpened_ = false;

    // open the popup window

    if(openFlag_ && (flags_ & ImGuiFileBrowserFlags_NoModal))
        SetNextWindowSize(ImVec2(700, 450));
    else
        SetNextWindowSize(ImVec2(700, 450), ImGuiCond_FirstUseEver);
    if(flags_ & ImGuiFileBrowserFlags_NoModal)
    {
        if(!BeginPopup(openLabel_.c_str()))
            return;
    }
    else if(!BeginPopupModal(openLabel_.c_str(), nullptr,
        flags_ & ImGuiFileBrowserFlags_NoTitleBar ? ImGuiWindowFlags_NoTitleBar : 0))
    {
        return;
    }
    isOpened_ = true;
    ScopeGuard endPopup([] { EndPopup(); });

    // display elements in pwd

#ifdef _WIN32
    char currentDrive = static_cast<char>(pwd_.c_str()[0]);
    char driveStr[] = { currentDrive, ':', '\0' };

    PushItemWidth(4 * GetFontSize());
    if(BeginCombo("##SelectDrive", driveStr))
    {
        ScopeGuard guard([&] { ImGui::EndCombo(); });
        for(int i = 0; i < 26; ++i)
        {
            if(!(drives_ & (1 << i)))
                continue;
            char driveCh = static_cast<char>('A' + i);
            char selectableStr[] = { driveCh, ':', '\0' };
            bool selected = currentDrive == driveCh;
            if(Selectable(selectableStr, selected) && !selected)
            {
                char newPwd[] = { driveCh, ':', '\\', '\0' };
                SetPwd(newPwd);
            }
        }
    }
    PopItemWidth();

    SameLine();
#endif

    int secIdx = 0, newPwdLastSecIdx = -1;
    for(auto &sec : pwd_)
    {
#ifdef _WIN32
        if(secIdx == 1)
        {
            ++secIdx;
            continue;
        }
#endif
        PushID(secIdx);
        if(secIdx > 0)
            SameLine();
        if(SmallButton(sec.u8string().c_str()))
            newPwdLastSecIdx = secIdx;
        PopID();
        ++secIdx;
    }

    if(newPwdLastSecIdx >= 0)
    {
        int i = 0;
        fs::path newPwd;
        for(auto &sec : pwd_)
        {
            if(i++ > newPwdLastSecIdx)
                break;
            newPwd /= sec;
        }
#ifdef _WIN32
        if(newPwdLastSecIdx == 0)
            newPwd /= "\\";
#endif
        SetPwd(newPwd);
    }

    SameLine();

    if(SmallButton("*"))
        SetPwd(pwd_);

    if(newDirNameBuf_)
    {
        SameLine();
        if(SmallButton("+"))
        {
            OpenPopup(openNewDirLabel_.c_str());
            (*newDirNameBuf_)[0] = '\0';
        }

        if(BeginPopup(openNewDirLabel_.c_str()))
        {
            ScopeGuard endNewDirPopup([] { EndPopup(); });

            InputText("name", newDirNameBuf_->data(), newDirNameBuf_->size()); SameLine();
            if(Button("ok") && (*newDirNameBuf_)[0] != '\0')
            {
                ScopeGuard closeNewDirPopup([] { CloseCurrentPopup(); });
                if(create_directory(pwd_ / newDirNameBuf_->data()))
                    SetPwd(pwd_);
                else
                    statusStr_ = "failed to create " + std::string(newDirNameBuf_->data());
            }
        }
    }

    // browse files in a child window

    float reserveHeight = GetItemsLineHeightWithSpacing();
    fs::path newPwd; bool setNewPwd = false;
    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory) && (flags_ & ImGuiFileBrowserFlags_EnterNewFilename))
        reserveHeight += GetItemsLineHeightWithSpacing();
    {
        BeginChild("ch", ImVec2(0, -reserveHeight), true,
            (flags_ & ImGuiFileBrowserFlags_NoModal) ? ImGuiWindowFlags_AlwaysHorizontalScrollbar : 0);
        ScopeGuard endChild([] { EndChild(); });

        for(auto &rsc : fileRecords_)
        {
            if (!rsc.isDir && typeFilters_.size() > 0 &&
                typeFilterIndex_ < int(typeFilters_.size()) &&
                !(rsc.extension == typeFilters_[typeFilterIndex_]))
                continue;

            if(!rsc.name.empty() && rsc.name[0] == '$')
                continue;

            const bool selected = selectedFilename_ == rsc.name;
            if(Selectable(rsc.showName.c_str(), selected, ImGuiSelectableFlags_DontClosePopups))
            {
                if(selected)
                {
                    selectedFilename_ = std::string();
                    (*inputNameBuf_)[0] = '\0';
                }
                else if(rsc.name != "..")
                {
                    if((rsc.isDir && (flags_ & ImGuiFileBrowserFlags_SelectDirectory)) ||
                       (!rsc.isDir && !(flags_ & ImGuiFileBrowserFlags_SelectDirectory)))
                    {
                        selectedFilename_ = rsc.name;
                        if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory))
                            strcpy(inputNameBuf_->data(), selectedFilename_.c_str());
                    }
                }
            }

            if(IsItemClicked(0) && IsMouseDoubleClicked(0) && rsc.isDir)
            {
                setNewPwd = true;
                newPwd = (rsc.name != "..") ? (pwd_ / rsc.name) : pwd_.parent_path();
            }
        }
    }

    if(setNewPwd)
        SetPwd(newPwd);

    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory) && (flags_ & ImGuiFileBrowserFlags_EnterNewFilename))
    {
        PushID(this);
        ScopeGuard popTextID([] { PopID(); });

        PushItemWidth(-1);
        if(InputText("", inputNameBuf_->data(), inputNameBuf_->size()))
            selectedFilename_ = inputNameBuf_->data();
        PopItemWidth();
    }

    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory))
    {
        if(Button(" ok ") && !selectedFilename_.empty())
        {
            ok_ = true;
            CloseCurrentPopup();
        }
    }
    else
    {
        if(selectedFilename_.empty())
        {
            if(Button(" ok "))
            {
                ok_ = true;
                CloseCurrentPopup();
            }
        }
        else if(Button("open"))
            SetPwd(pwd_ / selectedFilename_);
    }

    SameLine();

    int escIdx = GetIO().KeyMap[ImGuiKey_Escape];
    if(Button("cancel") || closeFlag_ ||
        ((flags_ & ImGuiFileBrowserFlags_CloseOnEsc) && IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && escIdx >= 0 && IsKeyPressed(escIdx)))
        CloseCurrentPopup();

    if(!statusStr_.empty() && !(flags_ & ImGuiFileBrowserFlags_NoStatusBar))
    {
        SameLine();
        Text("%s", statusStr_.c_str());
    }

    if (!typeFilters_.empty())
    {
        SameLine();
        PushItemWidth(8 * GetFontSize());
        Combo("##Filters", &typeFilterIndex_, typeFilters_.data(), int(typeFilters_.size()));
        PopItemWidth();
    }
}

inline bool ImGui::FileBrowser::HasSelected() const noexcept
{
    return ok_;
}

inline bool ImGui::FileBrowser::SetPwd(const fs::path &pwd)
{
    try
    {
        SetPwdUncatched(pwd);
        return true;
    }
    catch(const std::exception &err)
    {
        statusStr_ = std::string("last error: ") + err.what();
    }
    catch(...)
    {
        statusStr_ = "last error: unknown";
    }

    SetPwdUncatched(fs::current_path());
    return false;
}

inline fs::path ImGui::FileBrowser::GetSelected() const
{
    return pwd_ / selectedFilename_;
}

inline void ImGui::FileBrowser::ClearSelected()
{
    selectedFilename_ = std::string();
    (*inputNameBuf_)[0] = '\0';
    ok_ = false;
}

inline void ImGui::FileBrowser::SetTypeFilters(const std::vector<const char*> &typeFilters)
{
    typeFilters_ = typeFilters;
    typeFilterIndex_ = 0;
}

inline void ImGui::FileBrowser::SetPwdUncatched(const fs::path &pwd)
{
    fileRecords_ = { FileRecord{ true, "..", "[D] .." } };

    for(auto &p : fs::directory_iterator(pwd))
    {
        FileRecord rcd;

        if(p.is_regular_file())
            rcd.isDir = false;
        else if(p.is_directory())
            rcd.isDir = true;
        else
            continue;

        rcd.name = p.path().filename().string();
        if(rcd.name.empty())
            continue;

        rcd.extension = p.path().filename().extension().string();

        rcd.showName = (rcd.isDir ? "[D] " : "[F] ") + p.path().filename().u8string();
        fileRecords_.push_back(rcd);
    }

    std::sort(fileRecords_.begin(), fileRecords_.end(),
        [](const FileRecord &L, const FileRecord &R)
    {
        return (L.isDir ^ R.isDir) ? L.isDir : (L.name < R.name);
    });

    pwd_ = absolute(pwd);
    selectedFilename_ = std::string();
    (*inputNameBuf_)[0] = '\0';
}

#ifdef _WIN32

inline std::uint32_t ImGui::FileBrowser::GetDrivesBitMask()
{
    char rootName[10];
    DWORD mask = GetLogicalDrives();
    uint32_t ret = 0;
    for(int i = 0; i < 26; ++i)
    {
        if(!(mask & (1 << i)))
            continue;
        sprintf(rootName, "%c:\\", 'A' + i);
        UINT type = GetDriveTypeA(rootName);
        if(type == DRIVE_REMOVABLE || type == DRIVE_FIXED)
            ret |= (1 << i);
    }
    return ret;
}

#endif
