#include "MainWindow.hpp"
#include "UI/ui_MainWindow.h"

#include "ReflectionDataLink.hpp"
// #include "DesktopDataLink.hpp"
#include "CertificateProvider.hpp"

#include <QDateTime>
#include <QMessageBox>

#define SETUP_USERNAME(identifier)	"User-" identifier
#define REFLECTION_DATA_LINK_CAST(...)	dynamic_cast<ReflectionDataLink*>(__VA_ARGS__)

namespace /* anonymous */
{
	/// @brief Utility function to convert identifier string (each character is an identifier) to a string list.
	///	@param identifiers THe identifiers string.
	///	@return The username string list.
	QStringList IdentifiersToUsernames(const QString& identifiers)
	{
		QStringList usernames;
		for (const auto character : identifiers)
			usernames << QString("User-") + character;

		return usernames;
	}
}


MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, m_username(SETUP_USERNAME("A"))
	, m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);

	// Show the username.
	m_ui->username->setText("Username: " + m_username);

	// Setup the reflection network managers.
	setupReflectionNetworkManagers();

	// Setup UI connections.
	connect(m_ui->startButton, &QPushButton::clicked, this, [this] {m_pNetworkManagers.front()->pingPeers(); });
	connect(m_ui->onlineList, &QListWidget::itemClicked, this, &MainWindow::onUserSelected);
	connect(m_ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);
}

MainWindow::~MainWindow()
{
	// Quit all the threads.
    for (const auto& pNetworkManger : m_pNetworkManagers)
    {
        const auto pDataLink = REFLECTION_DATA_LINK_CAST(pNetworkManger->getDataLink());
        pDataLink->quit();
        pDataLink->wait();
    }


	delete m_ui;
}

void MainWindow::onUserSelected(const QListWidgetItem* pItem)
{
	// Validate the user.
	if (pItem->text() == m_username)
	{
		QMessageBox::warning(this, "Invalid user!", "The selected user is the current user!");
		return;
	}

	m_selectedUser = pItem->text();
	m_ui->selectedUser->setText("Selected user: " + pItem->text());
}

void MainWindow::onSendClicked()
{
	// Skip if the text input is empty.
	if (m_ui->textInput->text().isEmpty())
		return;

	// Validate the selected user.
	if (m_selectedUser.isEmpty())
	{
		QMessageBox::critical(this, "No User Selected!", "Please select a user before you can send messages!");
		return;
	}

	m_pNetworkManagers.front()->sendMessage(m_selectedUser.toStdString(), HiveCom::ToBytes(m_ui->textInput->text().toStdString()));
	m_ui->messageView->addItem(QString("%1\n%2").arg(m_username, m_ui->textInput->text()));
	m_ui->textInput->clear();
}

void MainWindow::setupReflectionNetworkManagers()
{
	// Setup the peer list.
	QVector<QPair<QString, QStringList>> peerList;
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("A"), IdentifiersToUsernames("BDNO")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("B"), IdentifiersToUsernames("ACFO")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("C"), IdentifiersToUsernames("B")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("D"), IdentifiersToUsernames("AE")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("E"), IdentifiersToUsernames("DFP")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("F"), IdentifiersToUsernames("BEG")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("G"), IdentifiersToUsernames("FHI")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("H"), IdentifiersToUsernames("GI")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("I"), IdentifiersToUsernames("GHJKL")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("J"), IdentifiersToUsernames("IK")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("K"), IdentifiersToUsernames("IJLM")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("L"), IdentifiersToUsernames("IK")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("M"), IdentifiersToUsernames("K")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("N"), IdentifiersToUsernames("AO")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("O"), IdentifiersToUsernames("ABN")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("P"), IdentifiersToUsernames("E")));

	// Setup peers.
	qsizetype index = 0;
	for (auto& [identifier, peers] : peerList)
	{
		m_pNetworkManagers.emplace_back(createReflectionNetworkManger(identifier.toStdString()));
		const auto pDataLink = REFLECTION_DATA_LINK_CAST(m_pNetworkManagers.back()->getDataLink());
		pDataLink->setPeers(std::move(peers));

        m_pNetworkManagers.back()->setOnMessageCallback([this, identifier = identifier](HiveCom::MessageFlag, const std::string& sender, const HiveCom::Bytes& bytes)
			{
				m_ui->messageView->addItem(QString("%1 (Reply)\n%2").arg(identifier, QString::fromStdString(HiveCom::ToString(bytes))));
			});

		m_usernameIndexMap[identifier] = index++;
	}

	// Setup connections.
	for (const auto& identifier : m_usernameIndexMap.keys())
	{
		const auto currentIndex = m_usernameIndexMap[identifier];
		const auto pDataLink = REFLECTION_DATA_LINK_CAST(m_pNetworkManagers[currentIndex]->getDataLink());
		const auto peers = pDataLink->getPeers();

		for (const auto& peer : peers)
			connect(pDataLink, &ReflectionDataLink::messageTransmission, REFLECTION_DATA_LINK_CAST(m_pNetworkManagers[m_usernameIndexMap[peer]]->getDataLink()), &ReflectionDataLink::onTransmissionReceived, Qt::QueuedConnection);
	}

	// Start all the threads.
	for (const auto& pNetworkManger : m_pNetworkManagers)
		REFLECTION_DATA_LINK_CAST(pNetworkManger->getDataLink())->start();
}

std::unique_ptr<HiveCom::NetworkManager> MainWindow::createReflectionNetworkManger(const std::string& identifier) const
{
	// Add the online list entry.
	m_ui->onlineList->addItem(QString::fromStdString(identifier));

	// Generate the certificate.
	const auto [certificate, kyberKey] = CertificateProvider::Instance().createCertificate(identifier);

	// Create and return the network manager.
	return std::make_unique<HiveCom::NetworkManager>(std::make_unique<ReflectionDataLink>(identifier, certificate, kyberKey));
}
