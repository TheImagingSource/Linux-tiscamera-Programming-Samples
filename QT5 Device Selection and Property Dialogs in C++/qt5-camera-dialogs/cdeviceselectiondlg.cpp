/*
 * Copyright  The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "cdeviceselectiondlg.h"
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <qdialog.h>
#include <glib.h>
#include <gst/gst.h>

/////////////////////////////////////////////////////////////////////
// Helper templates
//
template<typename T>
void DeleteItems(QComboBox *CBO)
{
	
	for( int i = 0; i<CBO->count(); i++ )
	{
		T *pFR = (T*)CBO->itemData(i).value<void*>();
		delete (pFR);
	}
	
	CBO->clear();
}

template<typename T>
T GetSelectedItemData(QComboBox *CBO)
{
	return (T)CBO->itemData(CBO->currentIndex()).value<void*>();
}



/**********************************************************************************
	In the constructor of the dialog the available cameras and devices are enumerated
	and listed in to combo boxes.
*/	
CDeviceSelectionDlg::CDeviceSelectionDlg(QWidget* parent ):QDialog(parent)
{
	CreateUI();

	EnumerateDevices();
}

CDeviceSelectionDlg::~CDeviceSelectionDlg()
{
	ClearComboBoxes();
}

void CDeviceSelectionDlg::EnumerateDevices()
{
	TcamProp* tcam;
	guint major;
    guint minor;
    guint micro;
    guint nano;
	
	char *Name = NULL;
	char *Identifier = NULL;
	char *Connection = NULL;
	
	gst_version(&major, &minor, &micro,&nano);
	
	
	tcam = (TcamProp*)gst_element_factory_make("tcamsrc", "tcam");
	if( tcam == NULL )
	{
		QMessageBox::warning(this,"TCam Capture","Can not instantiate tcamsrc.");
	}

	GSList *pSerials = tcam_prop_get_device_serials(tcam);
	
	while( pSerials )
	{
		DeviceDesc *Dev = new DeviceDesc;
		
		Dev->SerialNumber = (char*)pSerials->data;
		tcam_prop_get_device_info(tcam , Dev->SerialNumber.c_str() , &Name, &Identifier, &Connection);
		Dev->Name = Name;

		delete Name;
		delete Identifier;
		delete Connection;

		QVariant v;
		v.setValue((void*)Dev);
		
		cboCameras->addItem(std::string(  Dev->Name + " " + Dev->SerialNumber).c_str(),v );


		pSerials = pSerials->next;
	}
	
	cboCameras->setCurrentIndex(0);
	g_object_set(tcam,"serial",NULL,NULL);

	gst_object_unref(tcam); 

}

void CDeviceSelectionDlg::CreateUI()
{
	this->setWindowTitle("Device Selection");
	
	_MainLayout  = new QVBoxLayout();
	setLayout( _MainLayout );

	lblAction = new QLabel;
	lblAction->setAlignment(Qt::AlignCenter);
	
	QHBoxLayout *cam = new QHBoxLayout();
	
	QLabel *lblCam = new QLabel("Camera: ");
    cboCameras = new QComboBox();
	cboCameras->setMinimumWidth(250);
	QObject::connect(cboCameras, SIGNAL(currentIndexChanged(int)), this, SLOT(OncboCamerasChanced(int)));

	cam->addWidget(lblCam);
	cam->addWidget(cboCameras);

	_MainLayout->addLayout(cam);
	
	QHBoxLayout *Format = new QHBoxLayout();
	QLabel *lblFormat = new QLabel("Format: ");
	

    cboVideoFormats = new QComboBox();
    QObject::connect(cboVideoFormats, SIGNAL(currentIndexChanged(int)), this, SLOT(OncboVideoFormatsChanced(int)));

	Format->addWidget(lblFormat);
	Format->addWidget(cboVideoFormats);
	
	_MainLayout->addLayout(Format);

	QHBoxLayout *fps = new QHBoxLayout();
	QLabel *lblfps = new QLabel("Frame Rate: ");

	cboFrameRates = new QComboBox();

	fps->addWidget(lblfps);
	fps->addWidget(cboFrameRates);

	_MainLayout->addLayout(fps);

	_MainLayout->addWidget(lblAction);

	QHBoxLayout *buttons = new QHBoxLayout();

	//////////////////////////////////////////////////////////
	UpdateButton = new QPushButton(tr("Update"));
    connect(UpdateButton, SIGNAL(released()), this, SLOT(OnUpdateButton()));
    buttons->addWidget(UpdateButton);

	
	CancelButton = new QPushButton(tr("Cancel"));
    connect(CancelButton, SIGNAL(released()), this, SLOT(OnCancel()));
    buttons->addWidget(CancelButton);

	OKButton = new QPushButton(tr("OK"));
    connect(OKButton, SIGNAL(released()), this, SLOT(OnOK()));
    buttons->addWidget(OKButton);

	_MainLayout->addLayout(buttons);
}

/*******************************************************
	TODO: Implement something useful here.
*/
void CDeviceSelectionDlg::OnUpdateButton()
{
	lblAction->setText("Searching devices");
	lblAction->repaint();
	UpdateButton->setEnabled(false);
	OKButton->setEnabled(false);
	CancelButton->setEnabled(false);
	this->setCursor(Qt::WaitCursor);
	
	ClearComboBoxes();
	EnumerateDevices();

	lblAction->setText("");

	UpdateButton->setEnabled(true);
	OKButton->setEnabled(true);
	CancelButton->setEnabled(true);

	this->setCursor(Qt::ArrowCursor);
}

void CDeviceSelectionDlg::OnOK()
{
	printf("OnOK\n");
	
	DeviceDesc *DevDesc =  GetSelectedItemData<DeviceDesc*>(cboCameras);
	CVideoFormat *pVideoFormat = GetSelectedItemData<CVideoFormat*>(cboVideoFormats);
	CFrameRate *pFrameRate =  GetSelectedItemData<CFrameRate*>(cboFrameRates);

	_SelectedDevice.SerialNumber = DevDesc->SerialNumber;
	_SelectedDevice.Name = DevDesc->Name;
	_SelectedDevice.Format =  "";
	_SelectedDevice.Width = pVideoFormat->Width();
	_SelectedDevice.Height = pVideoFormat->Height();
	_SelectedDevice.fpsNominator = pFrameRate->_numerator;
	_SelectedDevice.fpsDeNominator = pFrameRate->_denominator;

	this->done(Accepted);
}



void CDeviceSelectionDlg::OnCancel()
{
	strcpy(_CapsString,"");
	this->done(Rejected);
}

void CDeviceSelectionDlg::ClearComboBoxes()
{
	
	DeleteItems<CFrameRate>(cboFrameRates);
	DeleteItems<CVideoFormat>(cboVideoFormats);
	DeleteItems<DeviceDesc>(cboCameras);

/*
	cboFrameRates->setEnabled(false);
	cboVideoFormats->setEnabled(false);
	cboCameras->setEnabled(false);
	*/
}

/******************************************************
	 New camera was selected
*/
void CDeviceSelectionDlg::OncboCamerasChanced(int Index)
{
	if( Index == -1 ) 
		return;
	
	lblAction->setText("Query video formats..");
	lblAction->repaint();
	
	DeleteItems<CFrameRate>(cboFrameRates);
	DeleteItems<CVideoFormat>(cboVideoFormats);

	DeviceDesc *DevDesc = GetSelectedItemData<DeviceDesc*>(cboCameras);
	
	EnumerateVideoFormats(DevDesc->SerialNumber);
	
	for( int i = 0; i < _VideoFormats.size(); i++ )
	{
		CVideoFormat *vf = new CVideoFormat(_VideoFormats.at(i));

		QVariant v;
		v.setValue((void*)vf );
		cboVideoFormats->addItem(vf->ToString(),v);
	}
	lblAction->setText("");

	return;
} 


////////////////////////////////////////////////////////////////////
//
bool CDeviceSelectionDlg::EnumerateVideoFormats(std::string SerialNumber )
{
	GstElement *TcamBin;

	_VideoFormats.clear();
	TcamBin = gst_element_factory_make("tcambin", "tcambin");

	if( TcamBin == NULL )
	{
		printf("Can not instantiate tcambin\n");
		//_LastErrorText = "Can not instantiate tcambin.";
		return false;
	}

	g_object_set(TcamBin,"serial",SerialNumber.c_str(),NULL);

	gst_element_set_state( TcamBin, GST_STATE_READY);
	
	GstPad *pSourcePad  = gst_element_get_static_pad(TcamBin,"src");

	GstCaps* pad_caps = gst_pad_query_caps( pSourcePad, NULL );

	if( pad_caps != NULL )
	{
		for (int i = 0; i < gst_caps_get_size (pad_caps); i++)
		{
			GstStructure *str = gst_caps_get_structure (pad_caps, i);
			if( strcmp( gst_structure_get_name(str),"ANY") != 0 )
			{
 				CVideoFormat vf(str);
				if( vf.isValid() )
					_VideoFormats.push_back(vf);
			}
		}
		gst_caps_unref(pad_caps);
	}

    g_object_unref(pSourcePad);
	gst_element_set_state( TcamBin, GST_STATE_NULL);

	gst_object_unref(TcamBin);

	return true;
}


////////////////////////////////////////////////////////////////////
//
void CDeviceSelectionDlg::OncboVideoFormatsChanced(int Index)
{
	
	if( Index == -1 ) 
		return;
	
	DeleteItems<CFrameRate>(cboFrameRates);

	CVideoFormat *pVF = GetSelectedItemData<CVideoFormat*>(cboVideoFormats);
	
	for( int i = 0; i < pVF->_framerates.size();i++ )
	{
		CFrameRate *pFR = new CFrameRate( pVF->_framerates.at(i));
		QVariant v;
		v.setValue((void*)pFR);

		cboFrameRates->addItem(pFR->ToString(),v);
	}
	
} 

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


CVideoFormat::CVideoFormat()
{
}

CVideoFormat::CVideoFormat(GstStructure *pStructure)
{
	const char * buffer = gst_structure_get_string (pStructure, "format" );
	_isValid = false;
	if( buffer == NULL )
	{
		strcpy( _format, "Unknown" );
	}
	else
	{
		if( strlen(buffer) != 0 && strlen(buffer) < 24)
		{
			strcpy( _format, buffer );
			strcpy(_type,gst_structure_get_name(pStructure));
			gst_structure_get_int (pStructure, "width", &_width);
			gst_structure_get_int (pStructure, "height", &_height);
		
			printf("%s %s %d %d\n", _type, _format, _width, _height );
			if( strcasecmp(_type,"video/x-bayer") == 0 )
			{
				strcpy(_type,"video/x-raw");
				strcpy(_format,"BGRA");
			}
			
			// Query the frame rate list of this video format.
			const GValue *value = gst_structure_get_value (pStructure, "framerate"); 
			if( GST_VALUE_HOLDS_LIST(value) )
			{
				for( int i = 0; i < gst_value_list_get_size(value); i++ )
				{
					const GValue *test = gst_value_list_get_value(value,i);
					if( G_IS_VALUE(test) )
					{
						_isValid = true;
						_framerates.push_back( CFrameRate(gst_value_get_fraction_numerator(test), gst_value_get_fraction_denominator(test) ));
					}
				}
			}
		}
	}
}

CVideoFormat::~CVideoFormat()
{
	_framerates.clear();
}

QString CVideoFormat::ToString()
{
	return QString("%1 (%2 x %3)").arg(_format, QString::number(_width), QString::number(_height) );
}

char* CVideoFormat::getGSTParameter(int FrameRateIndex)
{
	sprintf(_Parameter,"%s,format=BGRx,width=%d,height=%d,framerate=%d/%d",_type, _width, _height, _framerates[FrameRateIndex]._numerator,_framerates[FrameRateIndex]._denominator);
	printf("%s\n",_Parameter);
	return _Parameter;
}

bool CVideoFormat::operator!=(const CVideoFormat &Format)
{
	return _height != Format._height || _width != Format._width || strcmp(_format, Format._format) != 0 ;
}

bool CVideoFormat::operator==(const CVideoFormat &Format)
{
	return _height == Format._height && _width == Format._width && strcmp(_format, Format._format) == 0 ;
}

void CVideoFormat::debugprint()
{
	printf("Videoformat : %s %s %d %d\n",_type, _format, _width, _height);
}

/*
void CVideoFormat::getYAML( YAML::Emitter &config)
{
	config << YAML::Key << "width";
	config << YAML::Value << _width;
	config << YAML::Key << "height";
	config << YAML::Value << _height;
	config << YAML::Key << "type";
	config << YAML::Value << _type;
	config << YAML::Key << "format";
	config << YAML::Value << _format;
}

void CVideoFormat::setYAML(YAML::Node &config)
{
	_width = config["width"].as<int>();
	_height = config["height"].as<int>();
	strcpy(_type,  config["type"].as<std::string>().c_str());
	strcpy(_format,  config["format"].as<std::string>().c_str());
	
}
*/

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////



CFrameRate::CFrameRate( int numerator, int denominator)
{
	_numerator = numerator;
	_denominator = denominator;
}

QString CFrameRate::ToString()
{
	return QString("%1 / %2").arg( QString::number(_numerator), QString::number(_denominator) );
}

bool CFrameRate::operator!=(const CFrameRate &Rate)
{
	return _denominator != Rate._denominator || _numerator != Rate._numerator;
}

bool CFrameRate::operator==(const CFrameRate &Rate)
{
	return _denominator == Rate._denominator && _numerator == Rate._numerator;
}
/*
void CFrameRate::getYAML( YAML::Emitter &config)
{
	config << YAML::Key << "denominator";
	config << YAML::Value << _denominator;
	config << YAML::Key << "numerator";
	config << YAML::Value << _numerator;
}

void CFrameRate::setYAML(YAML::Node &config)
{
	_denominator = config["denominator"].as<int>();
	_numerator = config["numerator"].as<int>();

}
*/





