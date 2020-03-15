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
#include <QPrintPreviewDialog>
#endif
#endif
#endif


#ifdef Q_OS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
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
    connect(textEdit, &QTextEdit::currentCharFormatChanged,
            this, &TextEdit::currentCharFormatChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged,
            this, &TextEdit::cursorPositionChanged);
    setCentralWidget(textEdit);
    connect(textEdit, &QTextEdit::textChanged, this, &TextEdit::onTextChange);
    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupTextActions();

    {
        QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
        helpMenu->addAction(tr("About"), this, &TextEdit::about);
        helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    }

    QFont textFont("Helvetica");
    textFont.setStyleHint(QFont::SansSerif);
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
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}

void TextEdit::resizeEvent(QResizeEvent *event){
    modifyLabels();
}

void TextEdit::setupFileActions()
{
    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/filenew.png"));
    QAction *a = menu->addAction(newIcon,  tr("&New"), this, &TextEdit::fileNew);
    tb->addAction(a);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/fileopen.png"));
    a = menu->addAction(openIcon, tr("&Open..."), this, &TextEdit::fileOpen);
    a->setShortcut(QKeySequence::Open);
    tb->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &TextEdit::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb->addAction(actionSave);

    a = menu->addAction(tr("Save &As..."), this, &TextEdit::fileSaveAs);
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png"));
    a = menu->addAction(printIcon, tr("&Print..."), this, &TextEdit::filePrint);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    tb->addAction(a);

    const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png"));
    menu->addAction(filePrintIcon, tr("Print Preview..."), this, &TextEdit::filePrintPreview);

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png"));
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

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png"));
    actionCut = menu->addAction(cutIcon, tr("Cu&t"), textEdit, &QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &QTextEdit::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png"));
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

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/textbold.png"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &TextEdit::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/textitalic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &TextEdit::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/textunder.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Underline"), this, &TextEdit::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/textleft.png"));
    actionAlignLeft = new QAction(leftIcon, tr("&Left"), this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/textcenter.png"));
    actionAlignCenter = new QAction(centerIcon, tr("C&enter"), this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/textright.png"));
    actionAlignRight = new QAction(rightIcon, tr("&Right"), this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/textjustify.png"));
    actionAlignJustify = new QAction(fillIcon, tr("&Justify"), this);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    QActionGroup *alignGroup = new QActionGroup(this);
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
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void TextEdit::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*] - %2").arg(shownName, QCoreApplication::applicationName()));
    setWindowModified(false);
}

void TextEdit::fileNew()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFileName(QString());
    }
}

void TextEdit::fileOpen()
{
//    QFileDialog fileDialog(this, tr("Open File..."));
//    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
//    fileDialog.setFileMode(QFileDialog::ExistingFile);
//    fileDialog.setMimeTypeFilters(QStringList() << "text/html" << "text/plain");
//    if (fileDialog.exec() != QDialog::Accepted)
//        return;
//    const QString fn = fileDialog.selectedFiles().first();
//    if (load(fn))
//        statusBar()->showMessage(tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fn)));
//    else
//        statusBar()->showMessage(tr("Could not open \"%1\"").arg(QDir::toNativeSeparators(fn)));

    emit loadCRDTtoFile();
}

bool TextEdit::fileSave()
{
//    if (fileName.isEmpty())
//        return fileSaveAs();
//    if (fileName.startsWith(QStringLiteral(":/")))
//        return fileSaveAs();

//    QTextDocumentWriter writer(fileName);
//    bool success = writer.write(textEdit->document());
//    if (success) {
//        textEdit->document()->setModified(false);
//        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
//    } else {
//        statusBar()->showMessage(tr("Could not write to file \"%1\"")
//                                         .arg(QDir::toNativeSeparators(fileName)));
//    }
//    return success;

  emit saveCRDTtoFile();
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
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
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
    qreal pointSize = p.toFloat();
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
    if (a == actionAlignLeft)
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
    modifyLabels();
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
    qDebug() << " current: {"<< cursorPos << "} last: {" << lastCursorPos << "}";
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
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
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
    modifyLabels();

}

void TextEdit::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEdit::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);

    modifyLabels();
}

void TextEdit::modifyLabels(){
    int font;
    blockCursor = true;
    QTextCursor editorCurs = textEdit->textCursor();
    int editorPos = editorCurs.position();
    for(kk_cursor* c : cursors_.values()){
        editorCurs.setPosition(c->getGlobalPositon());
        font=editorCurs.charFormat().font().pointSize();
        c->setLabelsSize(font);
        c->moveLabels(textEdit->cursorRect(editorCurs));
    }
    editorCurs.setPosition(editorPos);
    blockCursor = false;
}

void TextEdit::applyRemoteChanges(QString operation, QString name, QString text, int globalPos, QString font) {
    QBrush color;
    //Blocco il cursore dell'editor.
    blockCursor = true;
    //Prelevo il cursore dell'editor.
    QTextCursor editorCurs = textEdit->textCursor();
    //editorCurs.charFormat().setBackground(Qt::white);
    //Prelevo il cursore remoto.
    kk_cursor* remoteCurs = cursors_.value(name);
    //Faccio dei controlli sulla fattibilità dell'operazione.
    globalPos = globalPos > lastLength ? lastLength : globalPos;
    globalPos = globalPos < 0 ? 0 : globalPos;
    //Se non esiste quel cursore lo creo e lo memorizzo.
    if(remoteCurs == nullptr) {
        //Creo il cursore per l'utente se non esiste.
        QLabel* qLbl = new QLabel(name, textEdit);
        QLabel* qLbl2 = new QLabel("|", textEdit);
        remoteCurs = new kk_cursor(globalPos);
        remoteCurs->setLabels(qLbl, qLbl2);
        //Seleziono randomicamente un colore dalla lista dei colori, controllo se era già stato usato.
        do{
            int index=rand() % colors_.size();
            color= colors_.at(index);
        }
        while (siteIdsColors_.values().contains(color));
        //Inserisco nella mappa dei colori.
        siteIdsColors_.insert(name,color);
        //Modifico la label.
        remoteCurs->setLabelsStyle(color, fontSize);
        remoteCurs->showLabels();
        cursors_.insert(name, remoteCurs);
    }
    // Muovo il cursore dell'editor.
    qDebug() << "Remote global:" << globalPos;
    editorCurs.setPosition(globalPos);
    // Eseguo l'operazione.
    if(operation == CRDT_INSERT) {
        //xxx qui devo mettere il font di quello che ha scritto, scrivere e poi rimettere il mio font vecchio

        QFont fontNuovo;
        QFont fontVecchio = textEdit->font();



        fontNuovo.fromString(font);

        QTextCharFormat format;
        format.setFont(fontNuovo);
        editorCurs.insertText(text,format);
        editorCurs.charFormat().setFont(fontVecchio);

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
    for(kk_cursor* c : cursors_.values()) {
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

    //Coloro (o decoloro) il mio testo se necessario
    //Se il mio siteId è cliccato coloro il difftext (solo se ho inserito), altrimenti seleziono il difftext e lo faccio bianco

    if(siteIdsClicked_.contains(name)){
        if(operation == CRDT_INSERT){ // Ho inserito del testo
            for(int i=0; i<text.length(); i++)
               siteIds_.value(name)->append(cursorPos-i);  // Aggiorno la lista di posizioni
            colorText(name);
    }
        else{ // Ho cancellato del testo
             for(int i=0; i<text.length(); i++)
                 siteIds_.value(name)->removeOne(cursorPos+i);
             clearColorText(name);
             colorText(name);

      }
    }

    lastCursorPos = cursorPos;
    editorCurs.setPosition(cursorPos);
    // Sblocco il cursore dell'editor.
    blockCursor = false;
}

void TextEdit::onTextChange() {
    // IMPORTANTE per le modifiche da remoto.
    if(blockCursor) return;
    // Restituisce il testo presente nell'editor.
    QString s = textEdit->toPlainText();

    if(lastLength - s.length() >= 1) {
        // Cancellato 1 o più
        if(isTextSelected){
            diffText=lastText.mid(selection_start, selection_end);
            qDebug() << "Testo cancellato: " << diffText << "start: {" << selection_start << "} end: {" << selection_end << "}";
            emit removeTextFromCRDT(selection_start, selection_end);
        }
        else{
            if (cursorPos < lastCursorPos) {
                diffText = lastText.mid(cursorPos, lastLength - s.length());
                qDebug() << "Testo cancellato:" << diffText << "current: {" << cursorPos << "} last: {" << lastCursorPos << "}";
                emit removeTextFromCRDT(cursorPos, lastCursorPos);
            } else {
                int diffLength = lastLength - s.length();
                diffText = lastText.mid(cursorPos, diffLength);
                qDebug() << "Testo cancellato:" << diffText << "current: {" << cursorPos << "} last: {" << lastCursorPos << "}";
                emit removeTextFromCRDT(cursorPos, cursorPos + diffLength);
            }
        }
    } else if(s.length() - lastLength >= 1) {
        // Inserito 1 o più
        //salva in diffText le cose nuove scritte
        diffText=s.mid(lastCursorPos, s.length() - lastLength);
        qDebug() << "Testo inserito: " << diffText << "current: {" << cursorPos << "} last: {" << lastCursorPos << "}";
        emit insertTextToCRDT(diffText, lastCursorPos);
    }
    // Aggiorno e muovo tutti i cursori sulla base dell'operazione.
    QTextCursor editorCurs = textEdit->textCursor();
    // Restituisce la posizione x,y di coordinate sullo schermo del tuo cursore
    int curPos_ = editorCurs.position();
    for (kk_cursor* c : cursors_.values()) {
        if (c->getGlobalPositon() > editorCurs.position()) {
            qDebug() << "Cursor global: " << c->getGlobalPositon();
            qDebug() << "Editor global: " << editorCurs.position();
            if (s.length() - lastLength > 0) {
                 c->setGlobalPositon(c->getGlobalPositon() + diffText.length());
            } else if(lastLength - s.length() > 0) {
                 c->setGlobalPositon(c->getGlobalPositon() - diffText.length());
            }
            editorCurs.setPosition(c->getGlobalPositon());
            c->moveLabels(textEdit->cursorRect(editorCurs));
            editorCurs.charFormat().setBackground(Qt::white);

        }
    }

    //Coloro (o decoloro) il mio testo se necessario
    //Se il mio siteId è cliccato coloro il difftext (solo se ho inserito), altrimenti seleziono il difftext e lo faccio bianco

    if(siteIdsClicked_.contains(mySiteId_)){
        if(s.length() - lastLength >= 1){ // Ho inserito del testo
            for(int i=0; i<s.length(); i++)
               siteIds_.value(mySiteId_)->append(curPos_-i);  // Aggiorno la lista di posizioni
            colorText(mySiteId_);
    }
        else{ // Ho cancellato del testo
             for(int i=0; i<s.length(); i++)
                 siteIds_.value(mySiteId_)->removeOne(curPos_+i);
             clearColorText(mySiteId_);
             colorText(mySiteId_);

      }
    }

    // Riporto il cursore dell'editor alla posizione di partenza.
    editorCurs.setPosition(curPos_);
    lastLength = s.length();
    lastText = s;

}

void TextEdit::updateSiteIdsMap(QString siteId, QSharedPointer<QList<int>> list){
//    if(siteIds_.contains(siteId))
//        siteIds_.remove(siteId);
//    siteIds_.insert(siteId,list);
    if(siteIdsClicked_.contains(siteId))
        clearColorText(siteId);
    else colorText(siteId);
}

void TextEdit::colorText(QString siteId){
    blockCursor=true;
    QTextCursor cursor = textEdit->textCursor();
    QTextCharFormat fmt=cursor.charFormat();
    QBrush color;
    //Se non ho ancora inserito il siteId nella mappa dei colori lo inserisco
    if(!siteIdsColors_.contains(siteId)){
        do{
            int index=rand() % colors_.size();
            color= colors_.at(index);
        }
        while (siteIdsColors_.values().contains(color));
        siteIdsColors_.insert(siteId,color);
    }

    fmt.setBackground(siteIdsColors_.value(siteId));
    int last = cursor.position();
    for(int pos : *siteIds_.value(siteId)){
        cursor.setPosition(pos);
        cursor.setPosition(pos+1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(fmt);
    }
    cursor.setPosition(last);
    textEdit->setTextCursor(cursor);

    siteIdsClicked_.push_front(siteId);
    blockCursor=false;
}

void TextEdit::clearColorText(QString siteId){
    blockCursor=true;
    QTextCursor cursor = textEdit->textCursor();
    QTextCharFormat fmt=cursor.charFormat();

    fmt.setBackground(Qt::white);
    int last = cursor.position();
    for(int pos : *siteIds_.value(siteId)){
        cursor.setPosition(pos);
        cursor.setPosition(pos+1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(fmt);
    }
    cursor.setPosition(last);
    textEdit->setTextCursor(cursor);

    //Elimino il siteId dalla lista di siteId cliccati (sto ricliccando)
    siteIdsClicked_.removeOne(siteId);

    blockCursor=false;
}


QTextEdit* TextEdit::getTextEdit(){
    return this->textEdit;
}

void TextEdit::setMySiteId(QString mySiteId){
    mySiteId_=mySiteId;
}





