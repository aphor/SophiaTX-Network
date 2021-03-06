/* (c) 2016, 2017 DECENT Services. For details refers to LICENSE.txt */
#include "stdafx.h"

#include "gui_wallet_global.hpp"
#include "richdialog.hpp"


#include "decent_button.hpp"
#include "decent_label.hpp"
#include "decent_line_edit.hpp"
#include "decent_text_edit.hpp"

#ifndef _MSC_VER
#include <graphene/chain/content_object.hpp>


#include <QIntValidator>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDateTime>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLocale>
#include <QInputMethod>
#include <QApplication>
#include <vector>
#include <string>
#endif


namespace gui_wallet
{
void PlaceInsideLabel(QWidget* pParent, QWidget* pChild)
{
   pParent->show();

   QHBoxLayout* pMainLayout = new QHBoxLayout;
   pMainLayout->addWidget(pChild);

   pMainLayout->setSizeConstraint(QLayout::SetFixedSize);
   pMainLayout->setSpacing(0);
   pMainLayout->setContentsMargins(0, 0, 0, 0);
   pParent->setLayout(pMainLayout);
}
//
// RatingWidget
//
RatingWidget::RatingWidget(QWidget* pParent)
   : QWidget(pParent)
   , m_bAutomation(false)
{
   QHBoxLayout* pMainLayout = new QHBoxLayout();

   m_arr_p_rate.resize(size);
   for (int index = 0; index < size; ++index)
   {
      m_arr_p_rate[index] = new DecentButton(this, DecentButton::StarIcon);
      m_arr_p_rate[index]->setCheckable(true);

      pMainLayout->addWidget(m_arr_p_rate[index]);

      QObject::connect(m_arr_p_rate[index], &QPushButton::toggled,
                       this, &RatingWidget::slot_rating);
   }

   pMainLayout->setSpacing(0);
   pMainLayout->setContentsMargins(0, 0, 0, 0);
   pMainLayout->setSizeConstraint(QLayout::SetFixedSize);

   setLayout(pMainLayout);
}

void RatingWidget::setRating(int rating)
{
   if (m_bAutomation)
      return;

   m_bAutomation = true;
   for (int index = 0; index < size; ++index)
   {
      QPushButton* pItem = m_arr_p_rate[index];
      if (index < rating)
         pItem->setChecked(true);
      else
         pItem->setChecked(false);
   }
   m_bAutomation = false;

   m_rating = rating;
   emit rated(rating);

   //
   // again, this is a trick to let styles refresh after a toggle
   //
   bool enabled = isEnabled();
   setDisabled(enabled);
   setEnabled(enabled);
}

void RatingWidget::slot_rating()
{
   for (int index = 0; index < size; ++index)
   {
      if (m_arr_p_rate[index] == sender())
         setRating(index + 1);
   }
}
//
// StackLayerWidget
//
StackLayerWidget::StackLayerWidget(QWidget* pParent)
: QWidget(pParent)
{
   QObject::connect(this, &StackLayerWidget::accepted,
                    this, &StackLayerWidget::closed);
}
//
// TransferWidget
//
TransferWidget::TransferWidget(QWidget* parent, QString const& userName/* = QString()*/)
   : StackLayerWidget(parent)
   , m_toUserName(userName)
{
   QVBoxLayout* mainLayout       = new QVBoxLayout();
   QVBoxLayout* lineEditsLayout  = new QVBoxLayout();
   QHBoxLayout* buttonsLayout    = new QHBoxLayout();
   
   DecentButton* ok = new DecentButton(this, DecentButton::DialogAction);
   ok->setText(tr("Send"));

   DecentButton* cancel = new DecentButton(this, DecentButton::DialogCancel);
   cancel->setText(tr("Back"));
   
   QObject::connect(ok, &QPushButton::clicked,
                    this, &TransferWidget::Transfer);
   QObject::connect(cancel, &QPushButton::clicked,
                    this, &StackLayerWidget::closed);
   
   DecentLineEdit* name = new DecentLineEdit(this, DecentLineEdit::DialogLineEdit);
   DecentLineEdit* amount = new DecentLineEdit(this, DecentLineEdit::DialogLineEdit);
   DecentLineEdit* memo = new DecentLineEdit(this, DecentLineEdit::DialogLineEdit);
   
   name->setPlaceholderText(tr("Account"));
   name->setAttribute(Qt::WA_MacShowFocusRect, 0);
   name->setText(m_toUserName);
   QObject::connect(name, &QLineEdit::textChanged,
                    this, &TransferWidget::nameChanged);
   
   amount->setPlaceholderText(tr("Amount"));
   amount->setAttribute(Qt::WA_MacShowFocusRect, 0);

   QDoubleValidator* dblValidator = new QDoubleValidator(0.0001, 100000, 4, this);
   dblValidator->setLocale(Globals::instance().locale());
   amount->setValidator(dblValidator);
   QObject::connect(amount, &QLineEdit::textChanged,
                    this, &TransferWidget::amountChanged);
   
   memo->setPlaceholderText(tr("Memo"));
   memo->setAttribute(Qt::WA_MacShowFocusRect, 0);
   QObject::connect(memo, &QLineEdit::textChanged,
                    this, &TransferWidget::memoChanged);
   
   lineEditsLayout->addWidget(name);
   lineEditsLayout->addWidget(amount);
   lineEditsLayout->addWidget(memo);

   buttonsLayout->addWidget(ok);
   buttonsLayout->addWidget(cancel);
   
   
   mainLayout->setContentsMargins(40, 10, 40, 10);
   mainLayout->addLayout(lineEditsLayout);
   mainLayout->addLayout(buttonsLayout);
   
   setLayout(mainLayout);

#ifdef _MSC_VER
   int height = style()->pixelMetric(QStyle::PM_TitleBarHeight);
   setWindowIcon(height > 32 ? QIcon(":/icon/images/windows_decent_icon_32x32.png")
      : QIcon(":/icon/images/windows_decent_icon_16x16.png"));
#endif
}

void TransferWidget::nameChanged(const QString & name)
{
   m_toUserName = name;
}

void TransferWidget::amountChanged(const QString & amount)
{
   m_amount = amount.toDouble();
}

void TransferWidget::memoChanged(const QString & memo)
{
   m_memo = memo;
}

void TransferWidget::Transfer()
{
   std::string message;
   
   if(m_fromUserName.isEmpty())
       m_fromUserName = Globals::instance().getCurrentUser().c_str();
   
   
   QString strAssetSymbol = Globals::instance().asset(0).m_str_symbol.c_str();

   try {
      QString run_str = "transfer "
      "\"" + m_fromUserName + "\" "
      "\"" + m_toUserName + "\" "
      "\"" + QString::number(m_amount) + "\""
      "\"" + strAssetSymbol + "\" "
      "\"" + m_memo + "\" "
      "\"true\"";
      Globals::instance().runTask(run_str.toStdString());
   } catch(const std::exception& ex){
      message = ex.what();
   }
   
   if (message.empty())
   {
      emit accepted();
      Globals::instance().slot_updateAccountBalance();
   }
   else
   {
      ShowMessageBox(tr("Error"), tr("Failed to transfer DCT"), message.c_str());
   }
}
//
// ImportKeyWidget
//
ImportKeyWidget::ImportKeyWidget(QWidget* parent)
: StackLayerWidget(parent)
{
   QObject::connect(this, &StackLayerWidget::accepted,
                    &Globals::instance(), &Globals::signal_keyImported);
   
   QVBoxLayout* mainLayout       = new QVBoxLayout();
   QVBoxLayout* lineEditsLayout  = new QVBoxLayout();
   QHBoxLayout* buttonsLayout    = new QHBoxLayout();
   
   DecentButton* ok = new DecentButton(this, DecentButton::DialogAction);
   ok->setText(tr("Ok"));
   DecentButton* cancel = new DecentButton(this, DecentButton::DialogCancel);
   cancel->setText(tr("Back"));
   
   QObject::connect(ok, &QPushButton::clicked,
                    this, &ImportKeyWidget::Import);
   QObject::connect(cancel, &QPushButton::clicked,
                    this, &StackLayerWidget::closed);
   
   DecentLineEdit* name = new DecentLineEdit(this, DecentLineEdit::DialogLineEdit);
   DecentLineEdit* key  = new DecentLineEdit(this, DecentLineEdit::DialogLineEdit);
   
   name->setPlaceholderText(tr("Account"));
   name->setAttribute(Qt::WA_MacShowFocusRect, 0);
   QObject::connect(name, &QLineEdit::textChanged,
                    this, &ImportKeyWidget::nameChanged);
   
   key->setPlaceholderText(tr("Key"));
   key->setAttribute(Qt::WA_MacShowFocusRect, 0);
   QObject::connect(key, &QLineEdit::textChanged,
                    this, &ImportKeyWidget::keyChanged);

   lineEditsLayout->addWidget(name);
   lineEditsLayout->addWidget(key);
   
   buttonsLayout->setSpacing(20);
   buttonsLayout->addWidget(ok);
   buttonsLayout->addWidget(cancel);

   mainLayout->setContentsMargins(40, 10, 40, 10);
   mainLayout->addLayout(lineEditsLayout);
   mainLayout->addLayout(buttonsLayout);
   
   setLayout(mainLayout);
}

void ImportKeyWidget::nameChanged(const QString & name)
{
   m_userName = name;
}

void ImportKeyWidget::keyChanged(const QString & key)
{
   m_key = key;
}

void ImportKeyWidget::Import()
{
   std::string message;

   try
   {
      QString csTaskStr = "import_key "
      "\"" + m_userName + "\" "
      "\"" + m_key + "\" ";
      auto result = Globals::instance().runTaskParse(csTaskStr.toStdString());
      bool tf = result.get<bool>();
      if(tf == false)
         message = "Invalid key";
   }
   catch (const std::exception& ex)
   {
      message = ex.what();
   }

   if (message.empty())
   {
      emit accepted();
   }
   else
   {
      ShowMessageBox(tr("Error"), tr("Cannot import key."), message.c_str());
   }
}
//
//UserInfoWidget
//
   
UserInfoWidget::UserInfoWidget(QWidget* parent,
                     const bool&    is_publishing_manager,
                     const bool     is_publishing_rights_received,
                     const QString& registrar,
                     const QString& name,
                     const QString& id
                     )
   : StackLayerWidget(parent)
{
   int labelCount = 0;
   QVBoxLayout* main_layout = new QVBoxLayout();
   main_layout->setSpacing(0);
   main_layout->setContentsMargins(0, 0, 0, 0);
   
   DecentLabel* registrarLabel = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Highlighted);
   ++labelCount;
   registrarLabel->setText(tr("Registrar - ") + registrar);
   main_layout->addWidget(registrarLabel);
   
   if(is_publishing_manager)
   {
      DecentLabel* managerIsPublishingLabel = new DecentLabel(this, DecentLabel::RowLabel);
      ++labelCount;
      managerIsPublishingLabel->setText((tr("Publishing manager")));
      main_layout->addWidget(managerIsPublishingLabel);
   }
   
   if(is_publishing_rights_received)
   {
      DecentLabel* isPublishingRightsReceivedLabel;
      if(labelCount % 2 == 0)
         isPublishingRightsReceivedLabel = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Highlighted);
      else
         isPublishingRightsReceivedLabel = new DecentLabel(this, DecentLabel::RowLabel);
      ++labelCount;
      isPublishingRightsReceivedLabel->setText((tr("Has rights to publish")));
      main_layout->addWidget(isPublishingRightsReceivedLabel);
   }
   
   DecentButton* backButton = new DecentButton(this, DecentButton::DialogCancel);
   backButton->setText("Back");
   QHBoxLayout* buttonLayout = new QHBoxLayout();
   buttonLayout->addWidget(backButton, Qt::AlignCenter);
   buttonLayout->setContentsMargins(0, 20, 0, 0);
   main_layout->addLayout(buttonLayout);
   QObject::connect(backButton, &QPushButton::clicked, this, &StackLayerWidget::closed);
   
   setWindowTitle(name + " (" + id + ")");
   setLayout(main_layout);
}
//
// ContentInfoWidget
//
ContentInfoWidget::ContentInfoWidget(QWidget* parent, const SDigitalContent& a_cnt_details)
   : StackLayerWidget(parent)
   , m_getItOrPay(GetIt)
   , m_URI(a_cnt_details.URI)
   , m_amount(a_cnt_details.price.getString().c_str())
{
   QGridLayout* main_layout = new QGridLayout();
   main_layout->setSpacing(0);
   main_layout->setContentsMargins(0, 0, 0, 15);

   int iRowIndex = 0;
   // Author
   //
   DecentLabel* labelAuthorTitle = new DecentLabel(this, DecentLabel::RowLabel);
   DecentLabel* labelAuthorInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Right);
   labelAuthorTitle->setText(tr("Author"));
   labelAuthorInfo->setText(QString::fromStdString(a_cnt_details.author));
   main_layout->addWidget(labelAuthorTitle, iRowIndex, 0);
   main_layout->addWidget(labelAuthorInfo, iRowIndex, 1);
   ++iRowIndex;

   // Expiration
   //
   DecentLabel* labelExpirationTitle = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Highlighted);
   DecentLabel* labelExpirationInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::HighlightedRight);
   QDateTime time = QDateTime::fromString(QString::fromStdString(a_cnt_details.expiration), "yyyy-MM-ddTHH:mm:ss");
   std::string e_str = CalculateRemainingTime(QDateTime::currentDateTime(), time);
   labelExpirationTitle->setText(tr("Expiration"));
   labelExpirationInfo->setText(QString::fromStdString(e_str));
   main_layout->addWidget(labelExpirationTitle, iRowIndex, 0);
   main_layout->addWidget(labelExpirationInfo, iRowIndex, 1);
   ++iRowIndex;

   // Uploaded
   //
   DecentLabel* labelUploadedTitle = new DecentLabel(this, DecentLabel::RowLabel);
   DecentLabel* labelUploadedInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Right);
   labelUploadedTitle->setText(tr("Uploaded"));
   labelUploadedInfo->setText(QString::fromStdString(a_cnt_details.created));
   main_layout->addWidget(labelUploadedTitle, iRowIndex, 0);
   main_layout->addWidget(labelUploadedInfo, iRowIndex, 1);
   ++iRowIndex;

   // Amount
   //
   DecentLabel* labelAmountTitle = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Highlighted);
   DecentLabel* labelAmountInfo  = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::HighlightedRight);
   QString str_price = a_cnt_details.price.getString().c_str();
   labelAmountTitle->setText(tr("Amount"));
   labelAmountInfo->setText(str_price);
   main_layout->addWidget(labelAmountTitle, iRowIndex, 0);
   main_layout->addWidget(labelAmountInfo, iRowIndex, 1);
   ++iRowIndex;

   // Size
   //
   DecentLabel* labelSizeTitle = new DecentLabel(this, DecentLabel::RowLabel);
   DecentLabel* labelSizeInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Right);
   labelSizeTitle->setText(tr("Size"));
   labelSizeInfo->setText(QString::number(a_cnt_details.size) + " MB");
   main_layout->addWidget(labelSizeTitle, iRowIndex, 0);
   main_layout->addWidget(labelSizeInfo, iRowIndex, 1);
   ++iRowIndex;
   
   // Times Bought
   //
   DecentLabel* labelTimesBoughtTitle = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Highlighted);
   DecentLabel* labelTimesBoughtInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::HighlightedRight);
   labelTimesBoughtTitle->setText(tr("Times Bought"));
   labelTimesBoughtInfo->setText(QString::number(a_cnt_details.times_bought));
   main_layout->addWidget(labelTimesBoughtTitle, iRowIndex, 0);
   main_layout->addWidget(labelTimesBoughtInfo, iRowIndex, 1);
   ++iRowIndex;
   
   std::string synopsis = a_cnt_details.synopsis;
   std::string title;
   std::string desc;
   
   graphene::chain::ContentObjectPropertyManager synopsis_parser(synopsis);
   title = synopsis_parser.get<graphene::chain::ContentObjectTitle>();
   desc = synopsis_parser.get<graphene::chain::ContentObjectDescription>();

   CommentWidget* pCommentWidget = new CommentWidget(this,
                                                     a_cnt_details.AVG_rating,
                                                     a_cnt_details.author,
                                                     a_cnt_details.URI,
                                                     desc);

   main_layout->addWidget(pCommentWidget, iRowIndex, 0, 1, 2);
   pCommentWidget->update();
   ++iRowIndex;
   
   DecentButton* getItButton = new DecentButton(this, DecentButton::DialogAction);
   DecentButton* cancelButton = new DecentButton(this, DecentButton::DialogCancel);
   getItButton->setText(tr("Get it!"));
   cancelButton->setText(tr("Back"));
   
   QObject::connect(getItButton, &QPushButton::clicked,
                    this, &ContentInfoWidget::ButtonWasClicked);
   QObject::connect(cancelButton, &QPushButton::clicked,
                    this, &StackLayerWidget::closed);

   QHBoxLayout* pButtonsLayout = new QHBoxLayout();
   // keeping buttons directly inside gridlayout cells leads to assymetry
   // so use hboxlayout
   pButtonsLayout->addWidget(getItButton);
   pButtonsLayout->addWidget(cancelButton);
   main_layout->addLayout(pButtonsLayout, iRowIndex, 0, 1, 2);
   
   setLayout(main_layout);

   setWindowTitle(QString::fromStdString(title));

#ifdef _MSC_VER
   int height = style()->pixelMetric(QStyle::PM_TitleBarHeight);
   setWindowIcon(height > 32 ? QIcon(":/icon/images/windows_decent_icon_32x32.png")
      : QIcon(":/icon/images/windows_decent_icon_16x16.png"));
#endif
}
   
void ContentInfoWidget::ButtonWasClicked()
{
   QPushButton* button = (QPushButton*) sender();
   if (m_amount == "Free" ||
       m_getItOrPay == Pay)
   {
      m_getItOrPay = GetIt;
      button->setText(tr("Get It!"));
      Buy();
   }
   else
   {
      m_getItOrPay = Pay;
      button->setText((tr("Pay") + " " + m_amount));
   }
}
   
void ContentInfoWidget::Buy()
{
   std::string downloadCommand = "download_content";
   downloadCommand += " " + Globals::instance().getCurrentUser();       // consumer
   downloadCommand += " \"" + m_URI + "\"";                             // URI
   downloadCommand += " \"\"";                                          // region_code
   downloadCommand += " true";                                          // broadcast

   std::string str_error;
   
   try
   {
      Globals::instance().runTask(downloadCommand);
   }
   catch(std::exception const& ex)
   {
      str_error = ex.what();
   }
   if (false == str_error.empty())
      ShowMessageBox("", tr("Failed to download content"), QString::fromStdString(str_error));
   
   emit accepted();
}

//
// ContentReviewWidget
//
ContentReviewWidget::ContentReviewWidget(QWidget* parent, const SDigitalContent& a_cnt_details)
: StackLayerWidget(parent)
, m_URI(a_cnt_details.URI)
{
   QGridLayout* main_layout = new QGridLayout();
   main_layout->setSpacing(0);
   main_layout->setContentsMargins(0, 0, 0, 15);
   
   int iRowIndex = 0;
   // Author
   //
   DecentLabel* labelAuthorTitle = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Highlighted);
   DecentLabel* labelAuthorInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::HighlightedRight);
   labelAuthorTitle->setText(tr("Author"));
   labelAuthorInfo->setText(QString::fromStdString(a_cnt_details.author));
   main_layout->addWidget(labelAuthorTitle, iRowIndex, 0);
   main_layout->addWidget(labelAuthorInfo, iRowIndex, 1);
   ++iRowIndex;
   
   // Purchased
   //
   DecentLabel* labelExpirationTitle = new DecentLabel(this, DecentLabel::RowLabel);
   DecentLabel* labelExpirationInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Right);
   labelExpirationTitle->setText(tr("Purchased"));
   labelExpirationInfo->setText(a_cnt_details.purchased_time.c_str());
   main_layout->addWidget(labelExpirationTitle, iRowIndex, 0);
   main_layout->addWidget(labelExpirationInfo, iRowIndex, 1);
   ++iRowIndex;
   
   // Amount
   //
   DecentLabel* labelAmountTitle = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Highlighted);
   DecentLabel* labelAmountInfo  = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::HighlightedRight);
   QString str_price = a_cnt_details.price.getString().c_str();
   labelAmountTitle->setText(tr("Amount"));
   labelAmountInfo->setText(str_price);
   main_layout->addWidget(labelAmountTitle, iRowIndex, 0);
   main_layout->addWidget(labelAmountInfo, iRowIndex, 1);
   ++iRowIndex;
   
   // Size
   //
   DecentLabel* labelSizeTitle = new DecentLabel(this, DecentLabel::RowLabel);
   DecentLabel* labelSizeInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Right);
   labelSizeTitle->setText(tr("Size"));
   labelSizeInfo->setText(QString::number(a_cnt_details.size) + " MB");
   main_layout->addWidget(labelSizeTitle, iRowIndex, 0);
   main_layout->addWidget(labelSizeInfo, iRowIndex, 1);
   ++iRowIndex;
   
   // Times Bought
   //
   DecentLabel* labelTimesBoughtTitle = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Highlighted);
   DecentLabel* labelTimesBoughtInfo = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::HighlightedRight);
   labelTimesBoughtTitle->setText(tr("Times Bought"));
   labelTimesBoughtInfo->setText(QString::number(a_cnt_details.times_bought));
   main_layout->addWidget(labelTimesBoughtTitle, iRowIndex, 0);
   main_layout->addWidget(labelTimesBoughtInfo, iRowIndex, 1);
   ++iRowIndex;

   std::string synopsis = a_cnt_details.synopsis;
   std::string title;
   std::string desc;
   
   graphene::chain::ContentObjectPropertyManager synopsis_parser(synopsis);
   title = synopsis_parser.get<graphene::chain::ContentObjectTitle>();
   desc = synopsis_parser.get<graphene::chain::ContentObjectDescription>();


   CommentWidget* pCommentWidget = new CommentWidget(this,
                                                     a_cnt_details.AVG_rating,
                                                     a_cnt_details.author,
                                                     a_cnt_details.URI,
                                                     desc,
                                                     Globals::instance().getCurrentUser());
   main_layout->addWidget(pCommentWidget, iRowIndex, 0, 1, 2);
   pCommentWidget->update();
   ++iRowIndex;

   DecentButton* cancelButton = new DecentButton(this, DecentButton::DialogCancel);
   cancelButton->setText(tr("Back"));

   QObject::connect(cancelButton, &QPushButton::clicked,
                    this, &StackLayerWidget::closed);

   main_layout->addWidget(cancelButton, iRowIndex, 0, 1, 2, Qt::AlignCenter);

   setLayout(main_layout);

   setWindowTitle(QString::fromStdString(title));
}
//
//CommentWidget
//
CommentWidget::CommentWidget(QWidget* pParent,
                             uint32_t content_average_rating,
                             const std::string& content_author,
                             const std::string& content_uri,
                             const std::string& content_description,
                             const std::string& feedback_author/* = std::string()*/)
: QWidget(pParent)
, m_pComment(new DecentTextEdit(this, DecentTextEdit::Info))
, m_pLabel(new DecentLabel(this, DecentLabel::RowLabel))
, m_pRatingWidget(new RatingWidget(this))
, m_pPreviousButton(new DecentButton(this, DecentButton::DialogTextButton))
, m_pNextButton(new DecentButton(this, DecentButton::DialogTextButton))
, m_pLeaveFeedbackButton(new DecentButton(this, DecentButton::DialogTextButton))
, m_content_average_rating(content_average_rating)
, m_content_author(content_author)
, m_content_uri(content_uri)
, m_content_description(content_description)
, m_feedback_author(feedback_author)
{
   m_pLeaveFeedbackButton->setText(tr("submit"));

   QGridLayout* pMainLayout = new QGridLayout;

   int iRowIndex = 0;

   DecentLabel* labelRatingInfoWrapper = new DecentLabel(this, DecentLabel::RowLabel, DecentLabel::Right);
   PlaceInsideLabel(labelRatingInfoWrapper, m_pRatingWidget);

   pMainLayout->addWidget(m_pLabel, iRowIndex, 0);
   pMainLayout->addWidget(labelRatingInfoWrapper, iRowIndex, 1);
   ++iRowIndex;

   pMainLayout->addWidget(m_pComment, iRowIndex, 0, 1, 2);
   ++iRowIndex;

   QHBoxLayout* pButtonsLayout = new QHBoxLayout;
   pButtonsLayout->addWidget(m_pPreviousButton);
   pButtonsLayout->addWidget(m_pNextButton);
   pButtonsLayout->addWidget(m_pLeaveFeedbackButton);
   pButtonsLayout->setSizeConstraint(QLayout::SetFixedSize);
   pButtonsLayout->setSpacing(0);
   pButtonsLayout->setContentsMargins(0, 0, 0, 0);

   pMainLayout->addLayout(pButtonsLayout, iRowIndex, 0, 1, 2, Qt::AlignCenter);
   ++iRowIndex;


   QObject::connect(m_pNextButton, &QPushButton::clicked,
                    this, &CommentWidget::slot_Next);
   QObject::connect(m_pPreviousButton, &QPushButton::clicked,
                    this, &CommentWidget::slot_Previous);
   QObject::connect(m_pLeaveFeedbackButton, &DecentButton::clicked,
                    this, &CommentWidget::submit);

   //pMainLayout->setSizeConstraint(QLayout::SetFixedSize);
   pMainLayout->setSpacing(0);
   pMainLayout->setContentsMargins(0, 0, 0, 0);

   setLayout(pMainLayout);
}

//Leave content feedback
void CommentWidget::submit()
{
   if (m_pRatingWidget->m_rating == 0)
      return;
   
   try
   {
      Globals::instance().runTaskParse("leave_rating_and_comment "
                                       "\"" + Globals::instance().getCurrentUser() + "\" "
                                       "\"" + m_content_uri + "\" "
                                       "\"" + std::to_string(m_pRatingWidget->m_rating) + "\" "
                                       "\"" + escape_string(m_pComment->toPlainText().toStdString() ) + "\" "
                                       "true" );
   }catch(...){}

   slot_Previous();
   slot_Next();
}
   
// update contentInfo
void CommentWidget::update()
{
   enum mode {view_description, leave_feedback, view_my_feedback, view_others_feedback};
   mode eMode;

   QString const cstrDescription = tr("description");

   QString strFeedbackAuthor;
   QString strFeedbackComment;
   uint32_t feedback_rating;

   std::string const c_str_hack = "hack_to_leave_comment";

   nlohmann::json feedback;
   try
   {
      std::string str_next_iterator = next_iterator();

      if (str_next_iterator != c_str_hack)
      {
         feedback = Globals::instance().runTaskParse("search_feedback "
                                                     "\"" + m_feedback_author + "\" "
                                                     "\"" + m_content_uri + "\" "
                                                     "\"" + str_next_iterator   + "\" " +
                                                     "2") ;

         if (feedback.size() > 0)
         {
            strFeedbackAuthor = feedback[0]["author"].get<std::string>().c_str();
            strFeedbackComment = feedback[0]["comment"].get<std::string>().c_str();
            feedback_rating = feedback[0]["rating"].get<int>();
         }
      }
   }catch(...){}

   int test_count = 0;

   if (m_feedback_author.empty())
   {
      if (next_iterator().empty())
         eMode = view_description;
      else
      {
         eMode = view_others_feedback;
         test_count = 1;
      }
   }
   else
   {
      if (next_iterator() == c_str_hack)
         eMode = leave_feedback;
      else if (next_iterator().empty())
      {
         eMode = view_description;

         if (feedback.empty())
            test_count = -1;
      }
      else
      {
         eMode = view_my_feedback;
         test_count = 1;
      }
   }

   if (-1 == test_count)
      set_next_comment(c_str_hack);
   else if (feedback.size() > test_count)
      set_next_comment(feedback[test_count]["id"].get<std::string>());
   else
      set_next_comment(std::string());

   m_pNextButton->setDisabled(is_last());
   m_pPreviousButton->setDisabled(is_first());

   if (is_last() && is_first())
   {
      m_pNextButton->hide();
      m_pPreviousButton->hide();
   }
   else
   {
      m_pNextButton->show();
      m_pPreviousButton->show();
   }

   m_pNextButton->setText(tr("next"));
   m_pPreviousButton->setText(tr("previous"));

   if (eMode == view_description)
   {
      m_pLabel->setText(tr("Average Rating"));
      m_pComment->setText(m_content_description.c_str());
      m_pComment->setPlaceholderText(QString());
      m_pRatingWidget->setRating(m_content_average_rating);

      if (false == m_feedback_author.empty())
      {
         m_pNextButton->setText(tr("feedback"));
         m_pPreviousButton->hide();
      }

      m_pLeaveFeedbackButton->hide();
      m_pRatingWidget->setDisabled(true);
      m_pComment->setReadOnly(true);
   }
   else if (eMode == view_my_feedback ||
            eMode == view_others_feedback)
   {
      if (eMode == view_my_feedback)
      {
         m_pLabel->setText(tr("Your own feedback"));
         m_pNextButton->hide();
         m_pPreviousButton->setText(cstrDescription);
      }
      else
         m_pLabel->setText(strFeedbackAuthor);

      m_pComment->setText(strFeedbackComment);
      m_pComment->setPlaceholderText(QString());
      m_pRatingWidget->setRating(feedback_rating);

      m_pLeaveFeedbackButton->hide();
      m_pRatingWidget->setDisabled(true);
      m_pComment->setReadOnly(true);
   }
   else
   {
      m_pNextButton->hide();
      m_pPreviousButton->setText(cstrDescription);
      m_pLabel->setText(tr("Leave your feedback here"));
      m_pComment->setText(QString());
      m_pComment->setPlaceholderText(tr("Any thoughts?"));
      m_pRatingWidget->setRating(0);

      m_pLeaveFeedbackButton->show();
      m_pRatingWidget->setEnabled(true);
      m_pComment->setReadOnly(false);
      m_pNextButton->hide();
   }
}

bool CommentWidget::is_first() const
{
   return m_iterators.empty();
}

bool CommentWidget::is_last() const
{
   return m_next_itr.empty();
}

void CommentWidget::set_next_comment(std::string const& last)
{
   m_next_itr = last;
}

std::string CommentWidget::next_iterator()
{
   if (!m_iterators.empty())
   {
      return m_iterators.back();
   }
   
   return std::string();
}

bool CommentWidget::slot_Next()
{
   if(is_last())
      return false;

   m_iterators.push_back(m_next_itr);
   update();
   
   return true;
}

bool CommentWidget::slot_Previous()
{
   if (is_first())
      return false;

   m_iterators.pop_back();
   update();
   
   return true;
}
// PasswordWidget
//
PasswordWidget::PasswordWidget(QWidget* pParent, eType enType)
: StackLayerWidget(pParent)
, m_enType(enType)
, m_pError(new QLabel(this))
{
   QLabel* pLabel = new QLabel(this);
   pLabel->setText(tr("The password must be limited to 50 characters"));
   DecentButton* pButton = new DecentButton(this, DecentButton::DialogAction);

   m_pError->hide();

   if (enType == eSetPassword)
      pButton->setText(tr("Set Password"));
   else
      pButton->setText(tr("Unlock"));

   DecentLineEdit* pEditPassword = new DecentLineEdit(this, DecentLineEdit::DialogLineEdit);
   pEditPassword->setEchoMode(QLineEdit::Password);
   pEditPassword->setAttribute(Qt::WA_MacShowFocusRect, 0);
   pEditPassword->setPlaceholderText(QString(tr("Password")));
   pEditPassword->setMaxLength(50);

   if (enType == eSetPassword)
      setWindowTitle(tr("Set Password"));
   else
   {
      pLabel->hide();
      setWindowTitle(tr("Unlock your wallet"));
   }

   int iRowIndex = 0;
   QGridLayout* pMainLayout = new QGridLayout;
   pMainLayout->addWidget(pLabel, iRowIndex++, 0, Qt::AlignCenter | Qt::AlignVCenter);
   pMainLayout->addWidget(m_pError, iRowIndex++, 0, Qt::AlignCenter | Qt::AlignVCenter);
   pMainLayout->addWidget(pEditPassword, iRowIndex++, 0, Qt::AlignCenter | Qt::AlignVCenter);
   pMainLayout->addWidget(pButton, iRowIndex++, 0, Qt::AlignCenter | Qt::AlignVCenter);

   pMainLayout->setSizeConstraint(QLayout::SetFixedSize);

   pMainLayout->setSpacing(20);
   pMainLayout->setContentsMargins(20, 20, 20, 20);

   QObject::connect(pEditPassword, &QLineEdit::returnPressed,
                    this, &PasswordWidget::slot_action);
   QObject::connect(pButton, &QPushButton::clicked,
                    this, &PasswordWidget::slot_action);
   QObject::connect(pEditPassword, &QLineEdit::textChanged,
                    this, &PasswordWidget::slot_set_password);

   setLayout(pMainLayout);
}

void PasswordWidget::slot_set_password(QString const& strPassword)
{
   m_strPassword = strPassword;
}

void PasswordWidget::slot_action()
{
   if (m_strPassword.isEmpty())
      return;

   if (m_enType == eSetPassword)
   {
      try
      {
         Globals::instance().runTask("set_password \"" + m_strPassword.toStdString() + "\"");
      }
      catch(...)
      {
         m_pError->setText(tr("Cannot set this password"));
         m_pError->show();
         return;
      }
   }

   try
   {
      Globals::instance().runTask("unlock \"" + m_strPassword.toStdString() + "\"");
   }
   catch(...)
   {
      m_pError->setText(tr("Cannot unlock the wallet"));
      m_pError->show();
      return;
   }

   emit accepted();
}


}  // end namespace gui_wallet
