#pragma once

#include "NetworkManager.hpp"

#include <QMainWindow>
#include <QHash>

// We'll be using std::vector<> to store the network managers.
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QListWidgetItem;

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

private slots:
	/// @brief This slot is called when the user clicks on an online user.
	///	@param pItem The item pointer.
	void onUserSelected(const QListWidgetItem* pItem);

	/// @brief This slot is called when the user clicks on the send button or once the text edit editing is finished.
	void onEditFinished();

	/// @brief This slot is called from data links when it requests to log something to the UI.
	///	@param content The content to log.
	void onLogRequested(const QString& content) const;

private:
	/// @brief Setup the reflection network managers.
	void setupReflectionNetworkManagers();

	/// @brief Create a new reflection network manager.
	///	@param identifier The network manager identifier.
	///	@return The network manager pointer.
	[[nodiscard]] std::unique_ptr<HiveCom::NetworkManager> createReflectionNetworkManger(const std::string& identifier) const;

private:
	QString m_username;
	QString m_selectedUser;

	std::vector<std::unique_ptr<HiveCom::NetworkManager>> m_pNetworkManagers;
	QHash<QString, qsizetype> m_usernameIndexMap;

	Ui::MainWindow* m_ui = nullptr;
};
