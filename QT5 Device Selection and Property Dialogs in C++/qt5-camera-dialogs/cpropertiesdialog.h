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


#ifndef CPROPERTYDLG_H
#define CPROPERTYDLG_H

#include <QtWidgets>
#include <tcamprop.h>


class PropVal
{
	public:
		PropVal();
		~PropVal();
		
		GValue _value;
		GValue _min;
		GValue _max;
		GValue _def;
		GValue _step;
		GValue _type;
		GValue _flags;
		GValue _category;
		GValue _group;
		
		void QueryProperty(TcamProp*, const char* PropertyName);
		
	private:
		void CopyGValue( GValue *dest, GValue *src);
		
};

class CPropertyControl : public QWidget
{
	Q_OBJECT
	
	public:
		std::string _PropertyName;
		int Property;

		CPropertyControl(const char* name, TcamProp *pTcamProp)
		{
			_PropertyName = name;
			_pTcamProp = pTcamProp;
			_Updating = false;
		}

		~CPropertyControl()
		{
		}
    
    	virtual void set_value(GValue *value)
		{
			printf("%s function set_value not implemented\n", _PropertyName.c_str());
		};

		void UpdateValue();

	protected:
		TcamProp *_pTcamProp;
		PropVal _pv;
		bool _Updating;
};

class CPropertyCtrl_int : public CPropertyControl
{
	Q_OBJECT

	public: 
		CPropertyCtrl_int( const char*  name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout);
		void set_value(GValue *value);
		
	private:
		QLabel *Label;
		QSlider *Slider;
		QLabel *Value;
		QHBoxLayout *hl;
		
		int get_value(int iPos);
		void show_value( int i);
		void set_property(const char *Property, const int Value );
		std::vector<uint> scale;
		
		void exposure_scale( int i_x_min,  int i_x_max, const int num_samples, const double s=8.0);
		
	private slots:
		void OnMovedSlider(int value);
		
};

class CPropertyCtrl_double : public CPropertyControl
{
	Q_OBJECT

	public: 
		CPropertyCtrl_double( const char*  name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout);
		void set_value(GValue *value);
		
	private:
		QLabel *Label;
		QSlider *Slider;
		QLabel *Value;
		QHBoxLayout *hl;
		std::vector<double> dscale;

		double  get_value(int iPos);
		void show_value( double i);
		
		void double_scale( double x_min,  double x_max, const int num_samples );
		void exposure_scale(const double x_min, const double x_max, const int num_samples=256, const double s=8.0);
		void set_property(const char *Property, const double Value );
		
	private slots:
		void OnMovedSlider(int value);
	
};

class CPropertyCtrl_boolean : public CPropertyControl
{
	Q_OBJECT

	public: 
		CPropertyCtrl_boolean( const char*  name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout);
		void set_value(GValue *value);
		
	private:
		QCheckBox *Check;
		void set_property(const char *Property, const bool  Value );

	private slots:
		void OnClickedCheckkBox(int); 
};


class CPropertyCtrl_button : public CPropertyControl
{
	Q_OBJECT

	public: 
		CPropertyCtrl_button( const char*  name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout);
		void set_value(GValue *value);
		
	private:
		QPushButton *Push;
		void set_property(const char *Property );

	private slots:
		void OnPushButton();
	
};


class CPropertyCtrl_enum : public CPropertyControl
{
	Q_OBJECT

	public: 
		CPropertyCtrl_enum( const char*  name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout);
		void set_value(GValue *value);

	private:
		QLabel *Label;
		QComboBox *Combobox;
		std::string getEnumProperty(const char *Property);
		void setEnumProperty(const char *Property, const char* value);
		GSList* getEnumPropertyValues(const char *Property);

	private slots:
		void OnCBOIndexChanged(int);
};



/////////////////////////////////////////////////////////////////////////////////////
// Dialog main window

class CPage
{
	public:
		CPage(std::string Title)
		{
			_Title = Title;
			_PageCreated = false;
			_QWidget = NULL;
			_PropertiesLayout = NULL;
		}
		
		std::string _Title;
		std::vector<std::string> _Categories;
		bool _PageCreated;
		QWidget *_QWidget;
		QVBoxLayout *_PropertiesLayout;
		void addCategory(std::string Category)
		{
			_Categories.push_back(Category);
		}
		
};
  
class CPropertiesDialog : public QDialog
{
  Q_OBJECT
  
  public:
      CPropertiesDialog( QWidget *parent);
      ~CPropertiesDialog();
      void SetCamera(TcamProp *_ptcambin);

  private slots:
	void updateproperties();
	void onOK();

      
  private:
    QTabWidget *_QTabs;
	std::vector <CPage> _Pages;
    
    //CGrabber *_pGrabber;
    std::vector<CPropertyControl*> _PropertySliders;
    void deleteControls();
	void CreatePageCategories();
    QVBoxLayout * findLayoutToAdd(const std::string Category);
    QVBoxLayout * getTabWidgetLayout(int page );
};

#endif // CPROPERTIESDIALOG_H
