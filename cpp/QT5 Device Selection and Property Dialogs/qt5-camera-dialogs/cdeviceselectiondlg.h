/*
 * Copyright 2015 bvtest <email>
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

#ifndef CDEVICESELECTIONDLG_H
#define CDEVICESELECTIONDLG_H

#include <QMainWindow>
#include <QtWidgets>
#include <QLabel>
#include <QAction>
#include <QtGui>
#include <QThread>
#include <QDialog>
#include <glib.h>
#include <gst/gst.h>

#include <tcamprop.h>
#include <vector>
#include <string>



/**********************************************************************
	CFrameRate
	Holds a single frame rate, consisting of numerator and denumerator.
	
*/
class CFrameRate
{
	public:
		CFrameRate()
		{
			_numerator = 0;
			_denominator = 1;
		}

		CFrameRate(const CFrameRate &src)
		{
			_numerator = src._numerator;
			_denominator = src._denominator;
		}

		
		CFrameRate( int numerator, int denominator);
		CFrameRate& operator=(const CFrameRate &src)
		{
			_numerator = src._numerator;
			_denominator = src._denominator;
			return *this;
		}
		
		bool operator!=(const CFrameRate &Rate);
		bool operator==(const CFrameRate &Rate);

		
		QString ToString();
		int _numerator;
		int _denominator;
		
		//void getYAML( YAML::Emitter &config);
		//void setYAML(YAML::Node &config);

};

////////////////////////////////////////////////////////////////
/*
	CVideo Format

	Hold information about width, height and format type of a video 
	format. 
	Also save a list of related frame rates 
*/
class CVideoFormat
{
	public:
		CVideoFormat();
		
		CVideoFormat(int width, int height)
		{
			_width = width;
			_height = height;
		}
		
		CVideoFormat(GstStructure *pStructure);
		CVideoFormat(const CVideoFormat &src)
		{
			Copy(src);
		};
		
		CVideoFormat& operator=(const CVideoFormat &src)
		{
			Copy(src);
			return *this;
		};
		 
		bool operator!=(const CVideoFormat &Format);
		bool operator==(const CVideoFormat &Format);
		
		
		
		~CVideoFormat();
		int Width(){return _width;};
		int Height(){return _height;};
		bool isValid(){return _isValid;};

		void debugprint();
		
		QString ToString();
		char* getGSTParameter(int FrameRateIndex);
		char _type[25];
		char _format[25];
		std::vector<CFrameRate> _framerates;
		//void getYAML( YAML::Emitter &config);
		//void setYAML(YAML::Node &config);
		
	private:
		int _width;
		int _height;
		char _Parameter[255];
		bool _isValid;
		
		void Copy( const CVideoFormat &src)
		{
			_width = src._width;
			_height = src._height;
			strcpy(_type,src._type);
			strcpy( _format,src._format);
			
			_framerates = src._framerates;
		}
	

};

/////////////////////////////////////////////////////////////////////


class CDeviceSelectionDlg : public QDialog
{
    Q_OBJECT

	public:
		CDeviceSelectionDlg(QWidget* parent );
		~CDeviceSelectionDlg();
	
		char* getCapString(){return _CapsString;};
		std::string getSerialNumber() {return _SelectedDevice.SerialNumber;};
		int getWidth() {return _SelectedDevice.Width;};
		int getHeight() {return _SelectedDevice.Height;};
		int getFPSNominator() {return _SelectedDevice.fpsNominator;};
		int getFPSDeNominator() {return _SelectedDevice.fpsDeNominator;};
		
	private slots:
		void OnCancel();
		void OnOK();
		void OnUpdateButton();
		void OncboCamerasChanced(int Index);
		void OncboVideoFormatsChanced(int Index);

	private:
		void ClearComboBoxes();
		void CreateUI();
		char _CapsString[2048];
		char _SerialNumber[2048];
		void DeleteFrameRatesCBO();
		void DeleteVideoFormatsCBO();
		void EnumerateDevices();
		bool EnumerateVideoFormats(std::string SerialNumber );

		struct DeviceDesc
		{
			std::string SerialNumber;
			std::string Name;
			std::string Format;
			int Width;
			int Height;
			int fpsNominator;
			int fpsDeNominator;
		};

		DeviceDesc _SelectedDevice;
		std::vector<DeviceDesc> _Devices;
		std::vector <CVideoFormat> _VideoFormats;

		
		
		QVBoxLayout *_MainLayout;
		QPushButton *CancelButton;
		QPushButton *OKButton;
		QPushButton *UpdateButton;
		QLabel *lblCameraName;
		QComboBox *cboCameras;
		QComboBox *cboVideoFormats;
		QComboBox *cboFrameRates;
		QLabel *lblAction;
};

#endif // CDEVICESELECTIONDLG_H
