#include "MainWindow.hpp"
#include "UI/ui_MainWindow.h"

#include "DesktopDataLink.hpp"

#include "HiveCom/CertificateAuthority.hpp"
#include "HiveCom/Kyber768.hpp"

#include <QDateTime>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, m_username(QDateTime::currentDateTime().toString("ddd-MMMM-d-yy-hh-mm-ss"))
	, m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);

	// Show the username.
	m_ui->username->setText("Username: " + m_username);

	// Create the root key pair.
	const auto keyPair = HiveCom::CertificateAuthority::Instance().createKeyPair();

	// Create the root key pair as trusted.
	HiveCom::CertificateAuthority::Instance().addTrustedPublicKey(
		HiveCom::ToFixedBytes<HiveCom::Dilithium3Key::PublicKeySize>(keyPair.getPublicKey()));

	// Create the kyber tool and keys.
	const HiveCom::Kyber768 kyber;
	const auto kemKey = kyber.generateKey();

	// Generate the certificate.
	const auto certificate = HiveCom::CertificateAuthority::Instance().createCertificate(
		m_username.toStdString(), kemKey.getPublicKey(), keyPair.getPrivateKey());

	// Set up the manager.
	m_pNetworkManager = std::make_unique<HiveCom::NetworkManager>(std::make_unique<DesktopDataLink>(m_username.toStdString(), certificate, kemKey));

	// Set up the required connections.
	const auto pDataLink = m_pNetworkManager->getDataLink();
	connect(dynamic_cast<DesktopDataLink*>(pDataLink), &DesktopDataLink::pingReceived, this, [this](DesktopDataLink::ClientType type, const QString& identifier)
		{
			if (m_ui->onlineList->findItems(identifier, Qt::MatchCaseSensitive).isEmpty())
				m_ui->onlineList->addItem(identifier);
		});

	// Setup UI connections.
	connect(m_ui->startButton, &QPushButton::clicked, this, [this]
		{
			m_pNetworkManager->pingPeers();
		});
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

