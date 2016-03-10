//
// Created by sergey on 04.03.16.
//

#include "TopWidget.h"
#include <QSplitter>
#include <QHBoxLayout>

TopWidget::TopWidget(QWidget* parent):
        QWidget(parent)
{
    CreateWidgets();
    CreateLayouts();
    CreateConnections();
}

void TopWidget::CreateWidgets()
{
    ppi = new Ppi(this);
    right = new QPlainTextEdit(this);
    right->setPlainText("Right");
}

void TopWidget::CreateLayouts()
{
    int index;
    QSplitter *  splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(ppi);
    splitter->addWidget(right);
    QHBoxLayout *  topLayout = new QHBoxLayout;
    topLayout->addWidget(splitter);
    setLayout(topLayout);
    index = splitter->indexOf(ppi);
    splitter->setCollapsible(index, false);
    splitter->setStretchFactor(index, 2);
    index = splitter->indexOf(right);
    splitter->setCollapsible(index, true);
    splitter->setStretchFactor(index, 1);
    right->setMaximumWidth(120);
    right->setMinimumWidth(80);
}

void TopWidget::CreateConnections()
{

}
