/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

#include "EditorSettings.h"
#include <QLineEdit>

namespace Ui {
class MainWindow;
}

class QFileDialog;
class Project;
class PackageDocument;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
	virtual void closeEvent(QCloseEvent * event);

private slots:
    void OnOpenFontManager();
    void OnOpenLocalizationManager();
    void OnShowHelp();
	
	void OnNewProject();
	void OnSaveProject();
	void OnSaveProjectAll();
    void OnOpenProject();
	void OnCloseProject();
	void OnExitApplication();
	
	void RecentMenuTriggered(QAction *recentAction);
	void RebuildRecentMenu();

    void SetBackgroundColorMenuTriggered(QAction* action);

    // Repack and Reload.
    void OnRepackAndReloadSprites();
    
    // Pixelization.
    void OnPixelizationStateChanged();
    
    void CurrentTabChanged(int index);
    void TabCloseRequested(int index);
    int CreateTabContent(DAVA::UIPackage *package);
    bool CloseTab(int index);
    bool CloseAllTabs();

    void OnOpenPackageFile(const QString &path);

private:
    void OpenProject(const QString &path);
    void SaveProject();
	bool CloseProject();
	
	void InitMenu();
    void SetupViewMenu();
	void UpdateMenu();
	void UpdateProjectSettings(const QString& filename);

	// Save/restore positions of DockWidgets and main window geometry
	void SaveMainWindowState();
	void RestoreMainWindowState();

	// Save the full project or changes only.
	void DoSaveProject(bool changesOnly);

    // Repack and reload sprites.
    void RepackAndReloadSprites();

    void UpdateSaveButtons();

    bool CheckAndUnlockProject(const QString& projectPath);
    
    PackageDocument *GetCurrentTabContent() const;
    int GetTabContent(const QString &fileName) const;
    
    PackageDocument *GetTabDocument(int index) const;

private:
    Ui::MainWindow *ui;

    // Background Frame Color menu actions.
    QList<QAction*> backgroundFramePredefinedColorActions;
    QAction* backgroundFrameUseCustomColorAction;
    QAction* backgroundFrameSelectCustomColorAction;

	bool screenChangeUpdate;
    QString screenShotFolder;
     
    Project *project;
    PackageDocument *activeDocument;
};

#endif // MAINWINDOW_H