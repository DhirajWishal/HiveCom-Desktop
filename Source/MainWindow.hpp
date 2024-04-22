#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "NetworkManager.hpp"

#include <QMainWindow>
#include <QHash>

// We'll be using std::vector<> to store the network managers.
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/// @brief Main window class.
///	This contains the main UI of the application.
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	/// @brief Explicit constructor.
	///	@param parent The parent widget pointer. Default is nullptr.
	explicit MainWindow(QWidget* parent = nullptr);

	/// @brief Destructor.
	~MainWindow();

private:
	/// @brief Create a new reflection network manager.
	///	@param identifier The network manager identifier.
	///	@return The network manager pointer.
	[[nodiscard]] std::unique_ptr<HiveCom::NetworkManager> createReflectionNetworkManger(const std::string& identifier) const;

private:
	QString m_username;
	std::vector<std::unique_ptr<HiveCom::NetworkManager>> m_pNetworkManagers;
	QHash<QString, qsizetype> m_usernameIndexMap;

	Ui::MainWindow* m_ui = nullptr;
};
#endif // MAINWINDOW_H
