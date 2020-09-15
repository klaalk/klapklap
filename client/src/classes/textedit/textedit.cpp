//
// Created by Klaus on 26/05/2019.
//

#include "textedit.h"
#include <QPlainTextEdit>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QLabel>
#include <QFontMetrics>
#include <QFont>
#include <QListWidgetItem>
#include <QTextCharFormat>

#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printer)
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#include <QPrinter>
#if QT_CONFIG(printpreviewdialog)
#include <QHBoxLayout>
#include <QPrintPreviewDialog>
#include <utility>
#endif
#endif
#endif


#ifdef Q_OS_MAC
const QString rsrcPath = ":/icon_set/png";
#else
const QString rsrcPath = ":/icon_set/png";
#endif



TextEdit::TextEdit(QWidget *parent)
    : QMainWindow(parent)
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setWindowState(Qt::WindowMaximized);
    setWindowTitle(QCoreApplication::applicationName());

    this->setMouseTracking(true);
    textEdit = new QTextEdit(this);
    //Collega funzioni nostre a funzioni di QTextEdit
    connect(textEdit, &QTextEdit::currentCharFormatChanged, this, &TextEdit::currentCharFormatChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &TextEdit::cursorPositionChanged);
    connect(textEdit, &QTextEdit::textChanged, this, &TextEdit::onTextChange);

    // Set layout
    QHBoxLayout *layout = new QHBoxLayout;
    setProperty("class", "crdtTextEdit");
    layout->addWidget(textEdit);
//    layout->addWidget(myWidget2);

    // Set layout in QWidget
    QWidget *window = new QWidget();
    window->setLayout(layout);

    // Set QWidget as the central layout of the main window
    setCentralWidget(window);
    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupTextActions();

    {
        QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
        helpMenu->addAction(tr("About"), this, &TextEdit::about);
        helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    }

    QFont textFont("MS Shell Dlg 2");
    textFont.setStyleHint(QFont::System);
    textFont.setPixelSize(10);
    textEdit->setFont(textFont);
    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    connect(textEdit->document(), &QTextDocument::modificationChanged,
            actionSave, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);


    setWindowModified(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isModified());
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

#ifndef QT_NO_CLIPBOARD
    actionCut->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    actionCopy->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TextEdit::clipboardDataChanged);
#endif

    textEdit->setFocus();
    setCurrentFileName(QString());

#ifdef Q_OS_MACOS
    // Use dark text on light background on macOS, also in dark mode.
    QPalette pal = textEdit->palette();
    pal.setColor(QPalette::Base, QColor(Qt::white));
    pal.setColor(QPalette::Text, QColor(Qt::black));
    textEdit->setPalette(pal);
#endif
}

void TextEdit::closeEvent(QCloseEvent *e)
{
    e->ignore();
    hide();
    emit openFileDialog();
}

void TextEdit::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event)
    modifyLabels();
}

void TextEdit::setupFileActions()
{
    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/file.png"));
    QAction *a = menu->addAction(newIcon,  tr("&New"), this, &TextEdit::fileNew);
    tb->addAction(a);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/folder.png"));
    a = menu->addAction(openIcon, tr("&Open..."), this, &TextEdit::fileOpen);
    a->setShortcut(QKeySequence::Open);
    tb->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/save.png"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &TextEdit::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb->addAction(actionSave);
    a = menu->addAction(tr("Save &As..."), this, &TextEdit::fileSaveAs);
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/print.png"));
    a = menu->addAction(printIcon, tr("&Print..."), this, &TextEdit::filePrint);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    tb->addAction(a);

    const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/print.png"));
    menu->addAction(filePrintIcon, tr("Print Preview..."), this, &TextEdit::filePrintPreview);

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/export.png"));
    a = menu->addAction(exportPdfIcon, tr("&Export PDF..."), this, &TextEdit::filePrintPdf);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);

    menu->addSeparator();
#endif

    a = menu->addAction(tr("&Quit"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void TextEdit::setupEditActions()
{
    QToolBar *tb = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/undo.png"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/redo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/scissors.png"));
    actionCut = menu->addAction(cutIcon, tr("Cu&t"), textEdit, &QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/copy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &QTextEdit::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/paste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Paste"), textEdit, &QTextEdit::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    tb->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::setupTextActions()
{
    QToolBar *tb = addToolBar(tr("Format Actions"));
    QMenu *menu = menuBar()->addMenu(tr("F&ormat"));

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/bold.png"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &TextEdit::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/italic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &TextEdit::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/underline.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Underline"), this, &TextEdit::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/left-align.png"));
    actionAlignLeft = new QAction(leftIcon, tr("&Left"), this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/center-align.png"));
    actionAlignCenter = new QAction(centerIcon, tr("C&enter"), this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/right-align.png"));
    actionAlignRight = new QAction(rightIcon, tr("&Right"), this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/justify.png"));
    actionAlignJustify = new QAction(fillIcon, tr("&Justify"), this);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    auto *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &TextEdit::textAlign);

    if (QApplication::isLeftToRight()) {
        alignGroup->addAction(actionAlignLeft);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignRight);
    } else {
        alignGroup->addAction(actionAlignRight);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignLeft);
    }
    alignGroup->addAction(actionAlignJustify);

    tb->addActions(alignGroup->actions());
    menu->addActions(alignGroup->actions());

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = menu->addAction(pix, tr("&Color..."), this, &TextEdit::textColor);
    tb->addAction(actionTextColor);

    tb = addToolBar(tr("Format Actions"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    comboStyle = new QComboBox(tb);
    tb->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    comboStyle->addItem("Ordered List (Roman lower)");
    comboStyle->addItem("Ordered List (Roman upper)");
    comboStyle->addItem("Heading 1");
    comboStyle->addItem("Heading 2");
    comboStyle->addItem("Heading 3");
    comboStyle->addItem("Heading 4");
    comboStyle->addItem("Heading 5");
    comboStyle->addItem("Heading 6");

    connect(comboStyle, QOverload<int>::of(&QComboBox::activated), this, &TextEdit::textStyle);

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    foreach (int size, standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textSize);

}

bool TextEdit::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        textEdit->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        textEdit->setPlainText(str);
    }

    setCurrentFileName(f);
    return true;
}

void TextEdit::loadCrdt(std::vector<std::list<KKCharPtr>> crdt)
{
    QTextCursor editorCurs = textEdit->textCursor();

    for(const auto& line : crdt) {
        for(const auto& charPtr : line) {
            applyRemoteChanges(CRDT_INSERT,
                               QString::fromStdString(charPtr->getSiteId()),
                               QChar::fromLatin1(charPtr->getValue()),
                               editorCurs.position()+1,
                               charPtr->getKKCharFont(),
                               charPtr->getKKCharColor()
                              );
        }
    }
}

void TextEdit::resetState() {
    blockCursor = false;
    isTextSelected = false;
    lastLength = 0;
    cursorPos=0;
    lastCursorPos=0;
    fontSize=0;
    selection_start=0;
    selection_end=0;
    lastText = "";
    diffText = "";
    fileName = "";
    cursors_.clear();
    siteIds_.clear();
    textEdit->clear();
}

bool TextEdit::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
            QMessageBox::warning(this, QCoreApplication::applicationName(),
                                 tr("The document has been modified.\n"
                                    "Do you want to save your changes?"),
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void TextEdit::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = fileName;
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1 - %2").arg(shownName, mySiteId_));
    setWindowModified(false);
}

void TextEdit::fileNew()
{
    emit openFileDialog();
}

void TextEdit::fileOpen()
{
    emit openFileDialog();
}

bool TextEdit::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();
    if (fileName.startsWith(QStringLiteral(":/")))
        return fileSaveAs();

    QTextDocumentWriter writer(fileName);
    bool success = writer.write(textEdit->document());
    if (success) {
        textEdit->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    } else {
        statusBar()->showMessage(tr("Could not write to file \"%1\"")
                                 .arg(QDir::toNativeSeparators(fileName)));
    }
    emit saveCRDTtoFile();
    return success;
}

bool TextEdit::fileSaveAs()
{
    QFileDialog fileDialog(this, tr("Save as..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes;
    mimeTypes << "application/vnd.oasis.opendocument.text" << "text/html" << "text/plain";
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.setDefaultSuffix("odt");
    if (fileDialog.exec() != QDialog::Accepted)
        return false;
    const QString fn = fileDialog.selectedFiles().first();
    setCurrentFileName(fn);
    return fileSave();
}

void TextEdit::filePrint()
{
#if QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);
    auto *dlg = new QPrintDialog(&printer, this);
    if (textEdit->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
        textEdit->print(&printer);
    delete dlg;
#endif
}

void TextEdit::filePrintPreview()
{
#if QT_CONFIG(printpreviewdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &TextEdit::printPreview);
    preview.exec();
#endif
}

void TextEdit::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    textEdit->print(printer);
#endif
}

void TextEdit::filePrintPdf()
{
#ifndef QT_NO_PRINTER
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    textEdit->document()->print(&printer);
    statusBar()->showMessage(tr("Exported \"%1\"")
                             .arg(QDir::toNativeSeparators(fileName)));
#endif
}

void TextEdit::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textSize(const QString &p)
{
    qreal pointSize = p.toDouble();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
        modifyLabels();
    }
}

void TextEdit::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();
    QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;

    switch (styleIndex) {
    case 1:
        style = QTextListFormat::ListDisc;
        break;
    case 2:
        style = QTextListFormat::ListCircle;
        break;
    case 3:
        style = QTextListFormat::ListSquare;
        break;
    case 4:
        style = QTextListFormat::ListDecimal;
        break;
    case 5:
        style = QTextListFormat::ListLowerAlpha;
        break;
    case 6:
        style = QTextListFormat::ListUpperAlpha;
        break;
    case 7:
        style = QTextListFormat::ListLowerRoman;
        break;
    case 8:
        style = QTextListFormat::ListUpperRoman;
        break;
    default:
        break;
    }

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    if (style == QTextListFormat::ListStyleUndefined) {
        blockFmt.setObjectIndex(-1);
        int headingLevel = styleIndex >= 9 ? styleIndex - 9 + 1 : 0; // H1 to H6, or Standard
        blockFmt.setHeadingLevel(headingLevel);
        cursor.setBlockFormat(blockFmt);

        int sizeAdjustment = headingLevel ? 4 - headingLevel : 0; // H1 to H6: +3 to -2
        QTextCharFormat fmt;
        fmt.setFontWeight(headingLevel ? QFont::Bold : QFont::Normal);
        fmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.mergeCharFormat(fmt);
        textEdit->mergeCurrentCharFormat(fmt);
    } else {
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        listFmt.setStyle(style);
        cursor.createList(listFmt);
    }

    cursor.endEditBlock();
}

void TextEdit::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}


void TextEdit::textAlign(QAction *a)
{
    QString alignment;
    if (a == actionAlignLeft){
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        alignment="left";
    }
    else if (a == actionAlignCenter){
         alignment="center";
        textEdit->setAlignment(Qt::AlignHCenter);
    }
    else if (a == actionAlignRight){
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
         alignment="right";
    }
    else if (a == actionAlignJustify){
        alignment="justify";
        textEdit->setAlignment(Qt::AlignJustify);
    }
    modifyLabels();
     emit(alignChange(alignment));
}

void TextEdit::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
    modifyLabels();
}

void TextEdit::cursorPositionChanged()
{
    // IMPORTANTE per le modifiche da remoto.
    if (blockCursor) return;
    //    alignmentChanged(textEdit->alignment());
    QTextList *list = textEdit->textCursor().currentList();
    QTextCursor cursor = textEdit->textCursor();
    QString text = cursor.selectedText();

    if (textEdit->textCursor().selectedText().size()!=0) {
        isTextSelected = true;
    } else {
        isTextSelected=false;
    }

    if (cursor.selectionStart() < cursor.selectionEnd()) {
        selection_end = cursor.selectionEnd();
        selection_start = cursor.selectionStart();
    } else if (cursor.selectionStart() > cursor.selectionEnd()) {
        selection_end = cursor.selectionStart();
        selection_start = cursor.selectionEnd();
    }

    lastCursorPos = cursorPos;
    cursorPos = cursor.position();
    qDebug() << "[cursorPositionChanged] Current: >"<< cursorPos << "< Last: >" << lastCursorPos << "<";
    if (list) {
        switch (list->format().style()) {
        case QTextListFormat::ListDisc:
            comboStyle->setCurrentIndex(1);
            break;
        case QTextListFormat::ListCircle:
            comboStyle->setCurrentIndex(2);
            break;
        case QTextListFormat::ListSquare:
            comboStyle->setCurrentIndex(3);
            break;
        case QTextListFormat::ListDecimal:
            comboStyle->setCurrentIndex(4);
            break;
        case QTextListFormat::ListLowerAlpha:
            comboStyle->setCurrentIndex(5);
            break;
        case QTextListFormat::ListUpperAlpha:
            comboStyle->setCurrentIndex(6);
            break;
        case QTextListFormat::ListLowerRoman:
            comboStyle->setCurrentIndex(7);
            break;
        case QTextListFormat::ListUpperRoman:
            comboStyle->setCurrentIndex(8);
            break;
        default:
            comboStyle->setCurrentIndex(-1);
            break;
        }
    } else {
        int headingLevel = textEdit->textCursor().blockFormat().headingLevel();
        comboStyle->setCurrentIndex(headingLevel ? headingLevel + 8 : 0);
    }
}

void TextEdit::clipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::about()
{
    QMessageBox::about(this, tr("About"), tr("This example demonstrates Qt's "
                                             "rich text editing facilities in action, providing an example "
                                             "document for you to experiment with."));
}

void TextEdit::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();

    if(cursor.hasSelection()) {
        qDebug()<<"[mergeFormatOnWordOrSelection] Formato da mettere nella selezione: "<< format.font().toString()<< " "<< format.foreground().color().name();
        emit selectionFormatChanged(cursor.selectionStart(), cursor.selectionEnd()-1, format);
    }
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}

void TextEdit::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
    fontSize=f.pointSize();
}

void TextEdit::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEdit::alignmentChanged(Qt::Alignment a)
{
    QString alignment;
    if (a & Qt::AlignLeft){
        actionAlignLeft->setChecked(true);
    }
    else if (a & Qt::AlignHCenter){
        actionAlignCenter->setChecked(true);
    }
    else if (a & Qt::AlignRight){
        actionAlignRight->setChecked(true);
    }
    else if (a & Qt::AlignJustify){
        actionAlignJustify->setChecked(true);
    }

    modifyLabels();
}

void TextEdit::alignmentRemoteChange(QString alignment)
{
    if (alignment=="left") {
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        actionAlignLeft->setChecked(true);
    }
    else if (alignment=="center") {
        textEdit->setAlignment(Qt::AlignHCenter);
        actionAlignCenter->setChecked(true);
    }
    else if (alignment=="right") {
         textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        actionAlignRight->setChecked(true);
    }
    else if (alignment=="justify") {
        textEdit->setAlignment(Qt::AlignJustify);
        actionAlignJustify->setChecked(true);
    }

    modifyLabels();
}

void TextEdit::modifyLabels(){
    int font;
    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

    QTextCursor editorCurs = textEdit->textCursor();
    int editorPos = editorCurs.position();

    for(KKCursor* c : cursors_.values()) {
        editorCurs.setPosition(c->getGlobalPositon());
        font = editorCurs.charFormat().font().pointSize();
        c->setLabelsSize(font);
        c->moveLabels(textEdit->cursorRect(editorCurs));
    }

    editorCurs.setPosition(editorPos);

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::applyRemoteChanges(const QString& operation, const QString& name, const QString& text, int globalPos, const QString& font, const QString& colorReceived) {

    //Blocco il cursore dell'editor.

    bool cursorBlocked=false;
    if (blockCursor)
        cursorBlocked = true;
    else blockCursor = true;

    //Prelevo il cursore dell'editor.
    QTextCursor editorCurs = textEdit->textCursor();

    //Prelevo il cursore remoto.
    KKCursor* remoteCurs = cursors_.value(name);

    //Faccio dei controlli sulla fattibilità dell'operazione.
    globalPos = globalPos > lastLength ? lastLength : globalPos;
    globalPos = globalPos < 0 ? 0 : globalPos;

    //Se non esiste quel cursore lo creo e lo memorizzo insieme alla label associata.
    //createCursorAndLabel(remoteCurs,name,globalPos);

    QBrush color;
    if(remoteCurs == nullptr) {
        //Creo il cursore per l'utente se non esiste.
        QLabel* qLbl = new QLabel(name, textEdit);
        QLabel* qLbl2 = new QLabel("|", textEdit);
        remoteCurs = new KKCursor(globalPos);
        remoteCurs->setLabels(qLbl, qLbl2);

        //Seleziono randomicamente un colore dalla lista dei colori, controllo se era già stato usato.
        color=selectRandomColor();

        // Modifico la label.
        remoteCurs->setLabelsStyle(color, fontSize);

        // Inserisco nella mappa dei colori.
        siteIdsColors_.insert(name, color);
        cursors_.insert(name, remoteCurs);
    }

    if (name != mySiteId_)
        remoteCurs->showLabels();

    // Muovo il cursore dell'editor.
    editorCurs.setPosition(globalPos);

    // Eseguo l'operazione.
    if(operation == CRDT_INSERT) {
        editorCurs.insertText(text);
    //Aggiorno formato
    for(int i=0;i<text.length();i++)
        singleCharFormatChange(globalPos + i,font,colorReceived);
     //Aggiorno la length.
     lastLength = lastLength + text.length();

    } else if(operation == CRDT_DELETE) {
        editorCurs.deleteChar();
     //Aggiorno la length.
     lastLength = lastLength - text.length();
    }

    // Aggiorno il cursore remoto
    remoteCurs->setGlobalPositon(editorCurs.position());

    // Muovo il cursore dopo l'aggiornamento
    remoteCurs->moveLabels(textEdit->cursorRect(editorCurs));

    // Aggiorno e muovo tutti i cursori sulla base dell'operazione.
    for(KKCursor* c : cursors_.values()) {
        if(c->getGlobalPositon() > globalPos && c!=remoteCurs){
            if(operation == CRDT_INSERT) {
                c->setGlobalPositon(c->getGlobalPositon()+(text.length()-1));
            } else if(operation == CRDT_DELETE) {
                c->setGlobalPositon(c->getGlobalPositon()-(text.length()-1));
            }
            editorCurs.setPosition(c->getGlobalPositon());
            c->moveLabels(textEdit->cursorRect(editorCurs));
        }
    }

    // Riporto il cursore dell'editor alla posizione di partenza.
    if(cursorPos >= globalPos){
        if(operation == CRDT_INSERT) {
            cursorPos = cursorPos +text.length();
        } else if(operation == CRDT_DELETE) {
            cursorPos = cursorPos -text.length();
        }
    }

    lastCursorPos = cursorPos;
    editorCurs.setPosition(cursorPos);

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::onTextChange() {

    // IMPORTANTE per le modifiche da remoto.
   if(blockCursor) return;
    // Restituisce il testo presente nell'editor.
    QString plainText = textEdit->toPlainText();

    if(lastLength - plainText.length() >= 1) {
        // Cancellato 1 o più
        if(isTextSelected){
            diffText=lastText.mid(selection_start, selection_end);
            qDebug() << "[onTextChange] Testo cancellato: " << diffText << "Selection start: >" << selection_start << "< Selection end: >" << selection_end << "<";
            emit removeTextFromCRDT(selection_start, selection_end);
        }
        else {
            if (cursorPos < lastCursorPos) {
                diffText = lastText.mid(cursorPos, lastLength - plainText.length());
                qDebug() << "[onTextChange] Testo cancellato (indietro):" << diffText << "Current: >" << cursorPos << "< Last: >" << lastCursorPos << "<";
                emit removeTextFromCRDT(cursorPos, lastCursorPos);
            } else {
                int diffLength = lastLength - plainText.length();
                diffText = lastText.mid(cursorPos, diffLength);
                qDebug() << "[onTextChange] Testo cancellato (avanti):" << diffText << "Current: >" << cursorPos << "< Last: >" << lastCursorPos << "<";
                emit removeTextFromCRDT(cursorPos, cursorPos + diffLength);
            }
        }

    } else if(plainText.length() - lastLength >= 1) {
        // Inserito 1 o più
        // Salva in diffText le cose nuove scritte
        diffText=plainText.mid(lastCursorPos, plainText.length() - lastLength);
        qDebug() << "[onTextChange] Testo inserito: " << diffText << "Current: >" << cursorPos << "< Last: >" << lastCursorPos << "<";
        emit insertTextToCRDT(diffText, lastCursorPos);
    }

    // Aggiorno e muovo tutti i cursori sulla base dell'operazione.
    QTextCursor editorCurs = textEdit->textCursor();

    // Restituisce la posizione x,y di coordinate sullo schermo del tuo cursore
    int curPos_ = editorCurs.position();
    for (KKCursor* c : cursors_.values()) {
        if (c !=nullptr && c->getGlobalPositon() > editorCurs.position()) {
            qDebug() << "[onTextChange] Cursor global: " << c->getGlobalPositon();
            qDebug() << "[onTextChange] Editor global: " << editorCurs.position();

            if (plainText.length() - lastLength > 0) {
                c->setGlobalPositon(c->getGlobalPositon() + diffText.length());
            } else if(lastLength - plainText.length() > 0) {
                c->setGlobalPositon(c->getGlobalPositon() - diffText.length());
            }

            editorCurs.setPosition(c->getGlobalPositon());
            c->moveLabels(textEdit->cursorRect(editorCurs));
        }
    }

    // Riporto il cursore dell'editor alla posizione di partenza.
    editorCurs.setPosition(curPos_);
    lastLength = plainText.length();
    lastText = plainText;

}

void TextEdit::updateSiteIdsMap(const QString& siteId, const QSharedPointer<QList<int>>& list){

    if(siteIds_.contains(siteId))
        siteIds_.remove(siteId);
    siteIds_.insert(siteId,list);

    if(siteIdsClicked_.contains(siteId))
        colorText(siteId);
    else if(clickedAny())
            clearColorText(siteId);
}

void TextEdit::siteIdClicked(const QString& siteId){
    if(siteIdsClicked_.contains(siteId)){  // 2 clicks
        clearColorText(siteId);
        siteIdsClicked_.removeOne(siteId);
    }
    else colorText(siteId);
}

void TextEdit::colorText(const QString& siteId){

    if(!siteIds_.contains(siteId)) return;

    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

    QTextCursor cursor = textEdit->textCursor();
    QTextCharFormat fmt;
    QBrush color;

    //Se non ho ancora inserito il siteId nella mappa dei colori lo inserisco
    if(!siteIdsColors_.contains(siteId)){
        color=selectRandomColor();
    }
    else color=siteIdsColors_.value(siteId);

    int last = cursor.position();

    for(int pos : *siteIds_.value(siteId)){
        cursor.setPosition(pos);
        if (cursor.charFormat().background()!=color){
            cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
            fmt=cursor.charFormat();
            fmt.setBackground(color);
            cursor.setCharFormat(fmt);
        }
    }
    cursor.setPosition(last);
    textEdit->setTextCursor(cursor);

    if(!siteIdsClicked_.contains(siteId))
        siteIdsClicked_.push_front(siteId);

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::clearColorText(const QString& siteId){

    if(!siteIds_.contains(siteId)) return;

    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

    QTextCursor cursor = textEdit->textCursor();
    QTextCharFormat fmt;

    int last = cursor.position();

    for(int pos : *siteIds_.value(siteId)){
        cursor.setPosition(pos);
        if (cursor.charFormat().background()!=Qt::white){
            cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
            fmt = cursor.charFormat();
            fmt.setBackground(Qt::white);
            cursor.setCharFormat(fmt);
        }
    }

    cursor.setPosition(last);
    textEdit->setTextCursor(cursor);

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}


void TextEdit::getCurrentFontAndColor(int pos, QString *font, QString *color){

    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

    QTextCursor curs = textEdit->textCursor();
    int oldPos=curs.position();
    curs.setPosition(pos);
    curs.movePosition(curs.Right, QTextCursor::KeepAnchor);
    *font= curs.charFormat().font().toString();
    *color= curs.charFormat().foreground().color().name();
    curs.setPosition(oldPos);

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::singleCharFormatChange(int remotePos, QString fontStr, QString colorStr){
    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

    QTextCursor editorCurs = textEdit->textCursor();
    int posIniziale=editorCurs.position();

    QFont fontNuovo;
    QColor coloreNuovo(colorStr);
    fontNuovo.fromString(fontStr);
    QTextCharFormat format;

    editorCurs.setPosition(remotePos);
    editorCurs.movePosition(editorCurs.Right, QTextCursor::KeepAnchor);

    format=editorCurs.charFormat();
    if(format.font()!=fontNuovo)
        format.setFont(fontNuovo);
    if(format.foreground()!=coloreNuovo)
        format.setForeground(coloreNuovo);

    editorCurs.setCharFormat(format);
    editorCurs.setPosition(posIniziale);

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::createCursorAndLabel(KKCursor* remoteCurs, const QString& name, int globalPos){

    QBrush color;
    if(remoteCurs == nullptr) {
        //Creo il cursore per l'utente se non esiste.
        QLabel* qLbl = new QLabel(name, textEdit);
        QLabel* qLbl2 = new QLabel("|", textEdit);
        remoteCurs = new KKCursor(globalPos);
        remoteCurs->setLabels(qLbl, qLbl2);

        //Seleziono randomicamente un colore dalla lista dei colori, controllo se era già stato usato.
        color=selectRandomColor();

        // Modifico la label.
        remoteCurs->setLabelsStyle(color, fontSize);

        // Inserisco nella mappa dei colori.
        siteIdsColors_.insert(name, color);
        cursors_.insert(name, remoteCurs);
    }

    if (name != mySiteId_)
        remoteCurs->showLabels();

}

QBrush TextEdit::selectRandomColor(){
    QBrush color;
    do{
        int index=rand() % colors_.size();
        color= colors_.at(index);
    }
    while (siteIdsColors_.values().contains(color));
    return color;
}

QTextEdit* TextEdit::getTextEdit(){
    return this->textEdit;
}
void TextEdit::setMySiteId(QString mySiteId){
    mySiteId_=std::move(mySiteId);
}
QString TextEdit::getMySiteId(){
    return this->mySiteId_;
}

bool TextEdit::clickedOne(const QString& siteId){
    return siteIdsClicked_.contains(siteId);
}
bool TextEdit::clickedAny(){
    return !siteIdsClicked_.isEmpty();
}

