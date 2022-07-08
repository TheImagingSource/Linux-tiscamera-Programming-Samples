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

#include "cpropertiesdialog.h"
#include <QTabWidget>


CPropertiesDialog::CPropertiesDialog(QWidget *parent) : QDialog(parent)
{
	_QTabs = new QTabWidget(this);
	QPushButton *Update = new QPushButton("Update");
	QPushButton *OK = new QPushButton("OK");

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(_QTabs);

	QHBoxLayout *Buttonslayout = new QHBoxLayout;


	Buttonslayout->addWidget(Update);
	Buttonslayout->addWidget(OK);
	layout->addLayout(Buttonslayout);


	connect(Update, SIGNAL(released()), this, SLOT(updateproperties()));
	connect(OK, SIGNAL(released()), this, SLOT(onOK()) );

	setLayout(layout);

	CreatePageCategories();
}


CPropertiesDialog::~CPropertiesDialog()
{
  g_print("Dialog Destructor called\n");
  deleteControls();
}


void CPropertiesDialog::CreatePageCategories()
{
	int i = 0;
	
	_Pages.push_back(CPage("Exposure"));
	_Pages[i].addCategory("Brightness");
	_Pages[i].addCategory("Exposure");
	_Pages[i].addCategory("Gain");
	_Pages[i].addCategory("Auto Functions ROI Control");
	i++;

	_Pages.push_back(CPage("Color"));
	_Pages[i].addCategory("Saturation");
	_Pages[i].addCategory("Hue");
	_Pages[i].addCategory("Whitebalance");
	i++;

	_Pages.push_back(CPage("Image"));
	_Pages[i].addCategory("Gamma");
	i++;

	_Pages.push_back(CPage("Lens"));
	_Pages[i].addCategory("Focus");
	_Pages[i].addCategory("Zoom");
	_Pages[i].addCategory("Iris");
	_Pages[i].addCategory("Special"); // For Focus one push
	i++;
	
	_Pages.push_back(CPage("Special"));
	_Pages[i].addCategory("Trigger Mode");
	_Pages[i].addCategory("Strobe Enable");
	_Pages[i].addCategory("GPIO");
	i++;
	
	_Pages.push_back(CPage("Partial Scan"));
	_Pages[i].addCategory("Offset Auto Center");
	i++;
	
	_Pages.push_back(CPage("Other"));
	_Pages[i].addCategory("Unknown");
	i++;
}

QVBoxLayout * CPropertiesDialog::findLayoutToAdd(const std::string Category)
{
	QVBoxLayout *FoundLayOut = NULL;
	
	for( int p = 0; p < _Pages.size(); p++ )
	{
		for( int c = 0; c < _Pages[p]._Categories.size(); c++ )
		{
			if( _Pages[p]._Categories[c] == Category )
			{
				FoundLayOut = getTabWidgetLayout(p);
			}
		}
	}
	
	// Unknown category, add property to last tab.
	if( FoundLayOut == NULL )
	{
		FoundLayOut = getTabWidgetLayout(_Pages.size() - 1);
	}

	return FoundLayOut;
}

QVBoxLayout * CPropertiesDialog::getTabWidgetLayout(int page )
{
	if(_Pages[page]._QWidget == NULL )
	{
		_Pages[page]._QWidget = new QWidget();

		_Pages[page]._PropertiesLayout = new QVBoxLayout();
		_Pages[page]._QWidget->setLayout( _Pages[page]._PropertiesLayout );
		_QTabs->addTab(_Pages[page]._QWidget, _Pages[page]._Title.c_str());	
	}
	return _Pages[page]._PropertiesLayout;
}

void CPropertiesDialog::deleteControls()
{
  // Clean up controls
  for( std::vector<CPropertyControl*>::iterator it = _PropertySliders.begin(); it != _PropertySliders.end(); it++ )
  {
    delete *it;
  }
  _PropertySliders.clear();
}

void CPropertiesDialog::SetCamera(TcamProp *_ptcambin )
{
	//setWindowTitle( _pCamera->_name);
	TcamProp *ptcambin = _ptcambin;
	deleteControls();

	if( ptcambin != NULL )
	{
		GSList* prop_names = tcam_prop_get_tcam_property_names(ptcambin);
		
		GSList* p = prop_names;
		do
		{
			PropVal pv;
			
			pv.QueryProperty(ptcambin, (gchar*)p->data);

			QVBoxLayout *LayoutToAdd = NULL;
			if(  g_value_get_string(&pv._group)!= NULL && strcmp( g_value_get_string(&pv._group), "" ) != 0 )
			{
				if( strcmp((const char*)(p->data), "Exposure Time (us)") == 0 )
					LayoutToAdd = findLayoutToAdd("Exposure"); // Workround for this property having "unknown" instead of exposure.
				else
					LayoutToAdd = findLayoutToAdd(g_value_get_string(&pv._group));
			}
			else		  
				if( strcmp( g_value_get_string(&pv._category), "" ) == 0 )
					LayoutToAdd = findLayoutToAdd("Focus"); // Workround for Focus properties having no category.
				else
					LayoutToAdd = findLayoutToAdd(g_value_get_string(&pv._category));

			
			CPropertyControl *pSld = NULL;
			if( strcmp( g_value_get_string(&pv._type) ,"integer") == 0)
			{
				pSld = new CPropertyCtrl_int((const char*)p->data,ptcambin,pv,LayoutToAdd);
			}

			if( strcmp( g_value_get_string(&pv._type) ,"double") == 0)
			{
				
				pSld = new CPropertyCtrl_double((const char*)p->data,ptcambin,pv,LayoutToAdd);
			}

			
			if( strcmp( g_value_get_string(&pv._type) ,"boolean") == 0)
			{
				
				pSld = new CPropertyCtrl_boolean((const char*)p->data,ptcambin,pv,LayoutToAdd);
			}

			if( strcmp( g_value_get_string(&pv._type) ,"button") == 0)
			{
				pSld = new CPropertyCtrl_button((const char*)p->data,ptcambin,pv,LayoutToAdd);
			}
			

			if( strcmp( g_value_get_string(&pv._type) ,"enum") == 0)
			{
				pSld = new CPropertyCtrl_enum((const char*)p->data,ptcambin,pv,LayoutToAdd);
			}

			
			
			if( pSld != NULL )
				_PropertySliders.push_back( pSld );
			else
				printf("WARNING : Unknown property \"%s\" of type \"%s\"\n", (char*)p->data, g_value_get_string(&pv._type));
			
			p = g_slist_next(p);
		}
		while (p != NULL);
	}
}


void CPropertiesDialog::updateproperties()
{
	for( int  i = 0; i < _PropertySliders.size(); i++ )
	{
		_PropertySliders[i]->UpdateValue();
	}
}

void CPropertiesDialog::onOK()
{
	this->done(Accepted);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// Propertycontrols

PropVal::PropVal():
	_value(),_min(),_max(),_def(),_step(),_type(),_flags(),_category(),_group()

{
}

PropVal::~PropVal()
{
	/*
	g_value_unset(&_value);
	g_value_unset(&_min);
	g_value_unset(&_max);
	g_value_unset(&_def);
	g_value_unset(&_step);
	g_value_unset(&_type);
	g_value_unset(&_flags);
	g_value_unset(&_group);
	g_value_unset(&_category);
	*/
}


void PropVal::QueryProperty(TcamProp* ptcambin, const char* PropertyName)
{
	GValue value = {};
	GValue min = {};
	GValue max = {};
	GValue def = {};
	GValue step = {};
	GValue type = {};
	GValue flags = {};
	GValue category = {};
	GValue group = {};

	tcam_prop_get_tcam_property(ptcambin, (gchar*)PropertyName, &value, &min,&max,&def,&step,&type,&flags,&category,&group);

	CopyGValue(&_value,&value);
	CopyGValue(&_min,&min);
	CopyGValue(&_max,&max);
	CopyGValue(&_def,&def);
	CopyGValue(&_step,&step);
	CopyGValue(&_type,&type);
	CopyGValue(&_flags,&flags);
	CopyGValue(&_category,&category);
	CopyGValue(&_group,&group);
}

void PropVal::CopyGValue( GValue *dest, GValue *src)
{
	g_value_unset(dest);
	g_value_init(dest,G_VALUE_TYPE(src));
	g_value_copy(src,dest );
}

///////////////////////////////////////////////////////
// Query the current property value and show it on the UI
void CPropertyControl::UpdateValue()
{
	PropVal pv;
	_Updating = true;
	pv.QueryProperty(_pTcamProp, _PropertyName.c_str());
	set_value(&pv._value);
	_Updating = false;
}






///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

CPropertyCtrl_int::CPropertyCtrl_int( const char* name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout) : CPropertyControl(name,	pTcamProp)
{

	Label = new QLabel( (gchar*)name);
	Slider = new QSlider(Qt::Horizontal);
	Value = new QLabel();
	QHBoxLayout *hl = new QHBoxLayout;

	if( strcmp(name,"Exposure") == 0  ||
		strcmp(name,"Exposure Time (us)") == 0 ||
		strcmp(name,"Exposure Auto Lower Limit") == 0 ||
		strcmp(name,"Exposure Auto Upper Limit") == 0 ||
		strcmp(name,"Exposure Min") == 0 ||
		strcmp(name,"Exposure Max") == 0 )
	{
		Slider->setMinimum( 0);
		Slider->setMaximum(99);
		exposure_scale(g_value_get_int(&pv._min),g_value_get_int(&pv._max),100);
	}
	else
	{
		Slider->setMinimum( g_value_get_int(&pv._min));
		Slider->setMaximum(g_value_get_int(&pv._max));
	}
	set_value(&pv._value);

	QObject::connect(Slider, SIGNAL(sliderMoved(int)), SLOT(OnMovedSlider(int)));
	QObject::connect(Slider, SIGNAL(valueChanged(int)), SLOT(OnMovedSlider(int)));

	hl->addWidget( Label,Qt::AlignLeft|Qt::AlignTop);
	hl->addWidget( Slider,Qt::AlignLeft|Qt::AlignTop);
	hl->addWidget( Value,Qt::AlignLeft|Qt::AlignTop);

	pLayout->addLayout(hl);
}

void CPropertyCtrl_int::set_value(GValue *value)
{
	int iValue = g_value_get_int(value);

	if( scale.size() > 0  )
	{
		for( int i = 0; i < scale.size() -1; i++ )
		{
			if( iValue >= scale[i] && iValue <= scale[i+1] )
			{
				Slider->setValue(i);
				show_value(scale[i]);
			}
		}
	}
	else
	{
		Slider->setValue(iValue);
		show_value(iValue);
	}
}

int CPropertyCtrl_int::get_value(int iPos)
{
	if(  scale.size() > 0  )
	{
		return scale[iPos];
	}
	return iPos;
}

void CPropertyCtrl_int::show_value( int i)
{
	char str[20];
	sprintf(str,"%d",i);

	if( Value != NULL )
	{
		Value->setAlignment(Qt::AlignRight|Qt::AlignCenter);
		Value->setText(str);
	}
}

void CPropertyCtrl_int::OnMovedSlider(int iPos)
{
	if( !_Updating )
		set_property(_PropertyName.c_str(),get_value(iPos));
	show_value( get_value(iPos));
}

void CPropertyCtrl_int::exposure_scale( int i_x_min,  int i_x_max, const int num_samples, const double s)
{
	float x_min = (float) i_x_min;
	float x_max = (float) i_x_max;

	float rangelen = log(x_max) - log(x_min);

	for( int i = 0; i<num_samples; i++)
	{
		float val = exp(log(x_min) + rangelen / num_samples * i);

		if( val < x_min) val=x_min;
		if( val > x_max) val=x_max;
        scale.push_back((uint)val);
	}
}

void CPropertyCtrl_int::set_property(const char *Property, const int Value )
{
	GValue val = {};
	char PropertyType[20];
	
	if(_pTcamProp != NULL )
	{
		strcpy( PropertyType, tcam_prop_get_tcam_property_type(_pTcamProp, (gchar*)Property));
		
		if( strcmp( PropertyType, "integer") == 0 )
		{
			g_value_init(&val, G_TYPE_INT);
			g_value_set_int(&val, Value);
		}
		else
		{
			if( strcmp( PropertyType, "boolean") == 0 )
			{
				g_value_init(&val, G_TYPE_BOOLEAN);
				if( Value == 1 )
					g_value_set_boolean(&val, true);
				else
					g_value_set_boolean(&val, false);
			}
			else
			{
				return;
			}
		}	
		
		if( !tcam_prop_set_tcam_property(_pTcamProp,(char*)Property , &val) )
		{
			printf("Failed to set %s\n", Property); 
		}
	}
	else
	{
		printf("Error: pipeline not built! Call start_camera() first.\n");
	}
}



///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

CPropertyCtrl_double::CPropertyCtrl_double( const char* name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout) : CPropertyControl(name,pTcamProp)
{

	Label = new QLabel( (gchar*)name);
	Slider = new QSlider(Qt::Horizontal);
	Value = new QLabel();
	QHBoxLayout *hl = new QHBoxLayout;

	Slider->setMinimum( 0);
	Slider->setMaximum(99);

	if( strcmp(name,"Exposure") == 0 ||
		strcmp(name,"Exposure Auto Lower Limit") == 0 ||
		strcmp(name,"Exposure Auto Upper Limit") == 0 )
	{
		exposure_scale(g_value_get_double(&pv._min),g_value_get_double(&pv._max),100);
	}
	else
		double_scale((double)g_value_get_double(&pv._min),(double)g_value_get_double(&pv._max),100);

	QObject::connect(Slider, SIGNAL(sliderMoved(int)), SLOT(OnMovedSlider(int)));
	QObject::connect(Slider, SIGNAL(valueChanged(int)), SLOT(OnMovedSlider(int)));

	hl->addWidget( Label,Qt::AlignLeft|Qt::AlignTop);
	hl->addWidget( Slider,Qt::AlignLeft|Qt::AlignTop);
	hl->addWidget( Value,Qt::AlignLeft|Qt::AlignTop);

	set_value(&pv._value);

	pLayout->addLayout(hl);
}

void CPropertyCtrl_double::set_value(GValue *value)
{
	double dValue = (double)g_value_get_double(value);

	if( dscale.size() > 0  )
	{
		for( int i = 0; i < dscale.size() -1; i++ )
		{
			if( dValue >= dscale[i] && dValue <= dscale[i+1] )
			{
				Slider->setValue(i);
				show_value(dscale[i]);
			}
		}
	}
}

double CPropertyCtrl_double::get_value(int iPos)
{
	if(  dscale.size() > 0  )
	{
		return dscale[iPos];
	}
	return (double)iPos;
}

void CPropertyCtrl_double::show_value( double i)
{
	char str[20];
	sprintf(str,"%f",i);

	if( Value != NULL )
	{
		Value->setAlignment(Qt::AlignRight|Qt::AlignCenter);
		Value->setText(str);
	}
}

void CPropertyCtrl_double::OnMovedSlider(int iPos)
{
	if( !_Updating )
		set_property(_PropertyName.c_str(),get_value(iPos));
	show_value( get_value(iPos));
}

/// Doubles linear.
void CPropertyCtrl_double::double_scale( double x_min,  double x_max, const int num_samples )
{

	double steps = (x_max - x_min) / double(num_samples);

	for( int i = 0; i<num_samples; i++)
	{
		double val = x_min + (double)i*steps;

		if( val < x_min) val=x_min;
		if( val > x_max) val=x_max;
        dscale.push_back(val);
	}
}

void CPropertyCtrl_double::exposure_scale( double x_min,  double x_max, const int num_samples, const double s)
{

	double rangelen = log((float)x_max) - log((float)x_min);

	for( int i = 0; i<num_samples; i++)
	{
		double val = exp( (float)( log((float)x_min) + rangelen / num_samples * i));

		if( val < x_min) val=x_min;
		if( val > x_max) val=x_max;
        dscale.push_back(val);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
void CPropertyCtrl_double::set_property(const char *Property, const double Value )
{
	GValue val = {};
	char PropertyType[20];
	
	if(_pTcamProp != NULL )
	{
		strcpy( PropertyType, tcam_prop_get_tcam_property_type(_pTcamProp, (gchar*)Property));
		
		if( strcmp( PropertyType, "double") == 0 )
		{
			g_value_init(&val, G_TYPE_DOUBLE);
			g_value_set_double(&val, Value);
		}
		
		if( !tcam_prop_set_tcam_property(_pTcamProp,(char*)Property , &val) )
		{
			printf("Failed to set %s\n", Property);
		}
	}
	else
	{
		printf("Error: pipeline not built! Call start_camera() first.\n");
	}
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

CPropertyCtrl_boolean::CPropertyCtrl_boolean( const char* name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout) : CPropertyControl(name,pTcamProp)
{

	Check = new QCheckBox(name );
	Check->setCheckState( g_value_get_boolean(&pv._value)? Qt::Checked : Qt::Unchecked );
	pLayout->addWidget( Check,Qt::AlignLeft|Qt::AlignTop);

	QObject::connect(Check, SIGNAL(stateChanged(int)), this, SLOT(OnClickedCheckkBox(int)));

}

void CPropertyCtrl_boolean::set_value(GValue *value)
{
	Check->setCheckState( g_value_get_boolean(value)? Qt::Checked : Qt::Unchecked );
}

void CPropertyCtrl_boolean::OnClickedCheckkBox(int value)
{
	if( Check->checkState() == Qt::Checked)
	{
		set_property(_PropertyName.c_str(), true);
	}
	else
	{
		set_property(_PropertyName.c_str(), false);
	}
}

void CPropertyCtrl_boolean::set_property(const char *Property, const bool  Value )
{
	GValue val = {};
	char PropertyType[20];
	
	if( _pTcamProp!= NULL )
	{
		strcpy( PropertyType, tcam_prop_get_tcam_property_type(_pTcamProp, (gchar*)Property));
		
		if( strcmp( PropertyType, "integer") == 0 )
		{
			g_value_init(&val, G_TYPE_INT);
			g_value_set_int(&val, Value);
		}
		else
		{
			if( strcmp( PropertyType, "boolean") == 0 )
			{
				g_value_init(&val, G_TYPE_BOOLEAN);
				if( Value == 1 )
					g_value_set_boolean(&val, true);
				else
					g_value_set_boolean(&val, false);
			}
			else
			{
				return;
			}
		}	
		
		if( !tcam_prop_set_tcam_property(_pTcamProp ,(char*)Property , &val) )
		{
			printf("Failed to set %s\n", Property);
		}
	}
	else
	{
		printf("Error: pipeline not built! Call start_camera() first.\n");
	}
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

CPropertyCtrl_button::CPropertyCtrl_button( const char* name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout) : CPropertyControl(name,pTcamProp)
{
	Push = new QPushButton( (gchar*)name );
	pLayout->addWidget( Push,Qt::AlignLeft|Qt::AlignTop);
	QObject::connect(Push, SIGNAL(released()), this, SLOT(OnPushButton()));

}

void CPropertyCtrl_button::set_value(GValue *value)
{
	// Nothing to do.
}

void CPropertyCtrl_button::OnPushButton()
{
  	set_property(_PropertyName.c_str());
}

void CPropertyCtrl_button::set_property(const char *Property )
{
	GValue val = {};
	char PropertyType[20];
	
	if( _pTcamProp != NULL )
	{
		strcpy( PropertyType, tcam_prop_get_tcam_property_type(_pTcamProp, (gchar*)Property));
		
		if( strcmp( PropertyType, "button") == 0 )
		{
			g_value_init(&val, G_TYPE_BOOLEAN);
			g_value_set_boolean(&val, true);
		
			if( !tcam_prop_set_tcam_property(_pTcamProp,(char*)Property , &val) )
			{
				printf("Failed to set %s\n", Property);
			}
 		}	
	}
	else
	{
		printf("Error: pipeline not built! Call start_camera() first.\n");
	}
}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

CPropertyCtrl_enum::CPropertyCtrl_enum( const char* name,TcamProp *pTcamProp, PropVal pv, QVBoxLayout *pLayout) : CPropertyControl(name,pTcamProp)
{
	Label = new QLabel( name );
	Combobox = new QComboBox();

	QHBoxLayout *hl = new QHBoxLayout;

	hl->addWidget( Label,Qt::AlignLeft|Qt::AlignTop);
	hl->addWidget( Combobox,Qt::AlignLeft|Qt::AlignTop);

	pLayout->addLayout(hl);

	std::string CurrentValue = getEnumProperty(_PropertyName.c_str());
	GSList* Values =  getEnumPropertyValues(_PropertyName.c_str());
	GSList* elem;
	int index = -1;
	for( elem = Values; elem; elem = elem->next )
	{
		index++;
		Combobox->addItem( (char*) elem->data );
		if( CurrentValue == (char*)elem->data)
			Combobox->setCurrentIndex(index);

	}
	QObject::connect(Combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCBOIndexChanged(int)));

}

//////////////////////////////////////////////////////////////////////////////////////
//
std::string CPropertyCtrl_enum::getEnumProperty(const char *Property)
{
	GValue value = {};
	GValue min = {};
	GValue max = {};
	GValue def = {};
	GValue step = {};
	GValue type = {};
	GValue flags = {};
	GValue category = {};
	GValue group = {};
	if( tcam_prop_get_tcam_property(_pTcamProp, (gchar*)Property, &value, &min,&max,&def,&step,&type,&flags,&category,&group))
	{
		return  g_value_get_string(&value);
	}
	return "";
}

void CPropertyCtrl_enum::setEnumProperty(const char *Property, const char* value)
{
	GValue val = {};
	char PropertyType[20];
	if( _pTcamProp != NULL )
	{
		g_value_init(&val,G_TYPE_STRING);
		g_value_set_string(&val, value);
		if( !tcam_prop_set_tcam_property(_pTcamProp ,(char*)Property , &val) )
		{
			printf("Failed to set %s\n", Property);
		}
	} 
}

GSList* CPropertyCtrl_enum::getEnumPropertyValues(const char *Property)
{
	GSList* Values = tcam_prop_get_tcam_menu_entries(_pTcamProp,Property);
	return Values;
}


void CPropertyCtrl_enum::set_value(GValue *value)
{
	// TODO
}

void CPropertyCtrl_enum::OnCBOIndexChanged(int)
{
	if( _Updating )
		return;
	QByteArray ba = Combobox->itemText(Combobox->currentIndex()).toLatin1();
	std::string Selected = ba.data();
	setEnumProperty(_PropertyName.c_str(), Selected.c_str() );
}






//#include "cpropertiesdialog.moc"
