#include "email.h"
#include "network.h"

Session_Config EmailLogger::setupConfig(LogConfig* logConf) {
  Session_Config config;
  config.server.host_name = logConf->smtpServer;
  config.server.port = logConf->smtpPort;
  config.login.email = logConf->smtpUser;
  config.login.password = logConf->smtpPass;
  config.login.user_domain = F("TC-pdu");

  return config;
}

void EmailLogger::process(LogProcess* state) {
  while (!(network.setupComplete && network.isConnected() && network.hasTime())) delay(500);

  LogConfig* logConf = config.getLog();
  if (logConf->smtpServer.length() <= 0) return;
  if (smtp == NULL) smtp = new SMTPSession();

  LogLine* msg = state->msg;
  smtp->setSystemTime(network.getEpochTime(), 0);

  Session_Config config = setupConfig(logConf);
  SMTP_Message message;
  char subject[100];
  snprintf(subject, sizeof(subject), "[%s] %s", msg->user, LOGTYPE[msg->type]);

  message.sender.name = F("TC-pdu");
  message.sender.email = logConf->smtpFrom;
  message.subject = subject;
  message.addRecipient(F("Admin"), logConf->smtpTo);
  message.text.content = msg->message;

  if (!smtp->connected()) {
    if (!smtp->connect(&config)) return;
  }

  MailClient.sendMail(smtp, &message);
  smtp->closeSession();
}

///// Global object

EmailLogger emailLog;
