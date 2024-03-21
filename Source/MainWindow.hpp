#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "HiveCom/NetworkManager.hpp"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private:
	QString m_username;
	std::unique_ptr<HiveCom::NetworkManager> m_pNetworkManager;

	Ui::MainWindow* m_ui = nullptr;
};
#endif // MAINWINDOW_H
