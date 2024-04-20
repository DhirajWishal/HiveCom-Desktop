#include "MainWindow.hpp"
#include "UI/ui_MainWindow.h"

#include "DesktopDataLink.hpp"

#include "Core/Kyber768.hpp"
#include "CertificateAuthority.hpp"

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
	const auto pDataLink = dynamic_cast<DesktopDataLink*>(m_pNetworkManager->getDataLink());
	connect(pDataLink, &DesktopDataLink::pingReceived, this, [this](DesktopDataLink::ClientType type, const QString& identifier)
		{
			QString typeString;
			switch (type) {
			case DesktopDataLink::ClientType::Desktop:
				typeString = "Desktop";
				break;
			case DesktopDataLink::ClientType::Mobile:
				typeString = "Mobile";
				break;
			case DesktopDataLink::ClientType::Embedded:
				typeString = "Embedded";
				break;
			}

			const auto uiIdentifier = QString("%1 (%2)").arg(identifier, typeString);
			if (m_ui->onlineList->findItems(uiIdentifier, Qt::MatchCaseSensitive).isEmpty())
				m_ui->onlineList->addItem(uiIdentifier);
		});

	connect(pDataLink, &DesktopDataLink::disconnected, this, [this](const QString& identifier)
		{
			for (int i = 0; i < m_ui->onlineList->count(); i++)
			{
				const auto item = m_ui->onlineList->item(i);
				if (item->text().startsWith(identifier))
				{
					delete m_ui->onlineList->takeItem(i);
					return;
				}
			}
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

