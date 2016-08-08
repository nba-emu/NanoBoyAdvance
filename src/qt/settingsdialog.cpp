///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Andreas Schulz
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include "settingsdialog.h"
#include "util.h"

#include <QFileDialog>
#include <QtGui>
#include <QMessageBox>


SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setModal(true);
    unsavedSettings = new QMap<QString, QVariant>;

    if (!qtUtil::appDir().exists())
    {
        if (!qtUtil::appDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)))
        {
            QMessageBox::critical(parent, tr("Permission error"), tr("The settings directory is not writable. Aborting."));
            return;
        }
    }
    centralLayout = new QVBoxLayout {this};
    setLayout(centralLayout);

    settingsLayout = new QVBoxLayout;

    useBiosFileCheckbox = new QCheckBox {tr("Use &Bios file"), this};
    QSettings settings {"nba-emu", "nanoboyadvance"};
    useBiosFileCheckbox->setChecked(settings.value("useBiosEnabled", false).toBool());

    connect(useBiosFileCheckbox, &QCheckBox::stateChanged,
            [this](int state)
            {
                if (state == Qt::Checked)
                {
                    // User clicked checkbox Ask for bios file.
                    if (QFile::exists(qtUtil::appDir().absolutePath() + QDir::separator() + "bios.bin"))
                    {
                        // User previously entered bios file. Reuse that.
                        (*unsavedSettings)["useBiosEnabled"] = true;
                    }
                    else
                    {
                        // User has never before entered bios file.
                        QString biosPath = QFileDialog::getOpenFileName(this, tr("Open Bios file"), QDir::homePath(), tr("Bios File (*.bin)"));
                        if (biosPath.isNull())
                        {
                            // User cancelled.
                            useBiosFileCheckbox->setChecked(false);
                        }
                        else
                        {
                            // User successfully selected file. Try to copy it to application directory.
                            if (!QFile::copy(biosPath, qtUtil::biosFilePath())) {
                                // Saving the bios failed.
                                useBiosFileCheckbox->setChecked(false);
                                QMessageBox::critical(this, tr("Copy error"), tr("Failed to save the bios."));
                            }
                            else
                            {
                                // The bios was successfully updated. Update the setings.
                                (*unsavedSettings)["useBiosEnabled"] = true;
                            }
                        }
                    }
                }
                else
                {
                    // User no longer wants to use bios.
                    (*unsavedSettings)["useBiosEnabled"] = false;
                }
            }
    );
    settingsLayout->addWidget(useBiosFileCheckbox);
    settingsLayout->addStretch();

    centralLayout->addLayout(settingsLayout);

    buttonBox = new QDialogButtonBox {QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    centralLayout->addWidget(buttonBox);
    initialized = true;
}

SettingsDialog::~SettingsDialog()
{
    delete unsavedSettings;
}

void SettingsDialog::accept()
{
    QSettings settings {"nba-emu", "nanoboyadvance"};
    for (auto i = unsavedSettings->constBegin(); i != unsavedSettings->constEnd(); ++i)
    {
        settings.setValue(i.key(), i.value());
    }
    QDialog::accept();
}

int SettingsDialog::exec()
{
    if (!initialized)
        return -1;
    return QDialog::exec();
}