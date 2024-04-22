#include "MainWindow.hpp"
#include "UI/ui_MainWindow.h"

#include "ReflectionDataLink.hpp"
// #include "DesktopDataLink.hpp"
#include "CertificateProvider.hpp"

#include <QDateTime>

#define SETUP_USERNAME(identifier)	"User-" identifier

namespace /* anonymous */
{
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

	// Setup the peer list.
	QVector<QPair<QString, QStringList>> peerList;
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("A"), IdentifiersToUsernames("BDNO")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("B"), IdentifiersToUsernames("ACFO")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("C"), IdentifiersToUsernames("B")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("D"), IdentifiersToUsernames("AE")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("E"), IdentifiersToUsernames("DFP")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("F"), IdentifiersToUsernames("BFG")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("G"), IdentifiersToUsernames("FHI")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("H"), IdentifiersToUsernames("GI")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("I"), IdentifiersToUsernames("GHJKL")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("J"), IdentifiersToUsernames("IK")));
	peerList.emplace_back(qMakePair<QString, QStringList>(SETUP_USERNAME("K"), IdentifiersToUsernames("JLM")));
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
		const auto pDataLink = dynamic_cast<ReflectionDataLink*>(m_pNetworkManagers.back()->getDataLink());
		pDataLink->setPeers(std::move(peers));

		m_usernameIndexMap[identifier] = index++;
	}

	// Setup connections.
	for (const auto& identifier : m_usernameIndexMap.keys())
	{
		const auto currentIndex = m_usernameIndexMap[identifier];
		const auto pDataLink = dynamic_cast<ReflectionDataLink*>(m_pNetworkManagers[currentIndex]->getDataLink());
		const auto peers = pDataLink->getPeers();

		for (const auto& peer : peers)
		{
			const auto pPeer = dynamic_cast<ReflectionDataLink*>(m_pNetworkManagers[m_usernameIndexMap[peer]]->getDataLink());
			connect(pDataLink, &ReflectionDataLink::messageTransmission, pPeer, &ReflectionDataLink::onTransmissionReceived, Qt::QueuedConnection);
			connect(pPeer, &ReflectionDataLink::messageTransmission, pDataLink, &ReflectionDataLink::onTransmissionReceived, Qt::QueuedConnection);
		}
	}

	// Start all the threads.
	for (const auto& pNetworkManger : m_pNetworkManagers)
		dynamic_cast<ReflectionDataLink*>(pNetworkManger->getDataLink())->start();

	// Setup UI connections.
	connect(m_ui->startButton, &QPushButton::clicked, this, [this]
		{
			m_pNetworkManagers.front()->pingPeers();
		});
}

MainWindow::~MainWindow()
{
	delete m_ui;
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
