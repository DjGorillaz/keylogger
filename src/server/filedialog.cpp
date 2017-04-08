#include "filedialog.h"
#include "ui_filedialog.h"

FileDialog::FileDialog(QWidget *parent) :
    QDialog(parent),
    mask(0),
    ui(new Ui::FileDialog)
{
    ui->setupUi(this);
}

FileDialog::~FileDialog()
{
    delete ui;
}

quint16& FileDialog::getFileMask()
{
    mask |= ui->chromePass->checkState() ? Files::ChromePass : 0 ;
    return mask;
}

QString& FileDialog::getFileString()
{
   files = ui->lineEdit->text();
   return files;
}
