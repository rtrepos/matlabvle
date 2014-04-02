
#include <cassert>
#include <vle/utils.hpp>
#include <vle/manager/Run.hpp>
#include <vle/manager/Manager.hpp>
#include <vle/manager/VLE.hpp>
#include <vle/manager/Types.hpp>
#include <vle/vpz/Vpz.hpp>
#include <vle/graph/Model.hpp>

#include "convert.h"
#include "matlabvle.h"

#include "mex.h"
#include "matrix.h"
#include "class_handle.hpp"
#include <iostream>
#include <stdlib.h>



#if !(defined _WIN32 || defined _WIN64) // needed to compile on linux.
#include <string.h>
#endif

using namespace vle;
using namespace utils;
//using namespace std;


static bool thread_init=false;

class Vle
{
public:
    
    Vle()
    {
        setFileName((char*)"");
        setPackageName((char*)"");
        init();
        setMethodsList();
    };
    
    Vle(char* pkgname, char* filename)
    {
        setFileName(filename);
        setPackageName(pkgname);
        init();
        openFile();
        setMethodsList();
    };
    
    
    void setName(const char name[100],const char name_type){
        //std::cout << "name type :" << name_type << std::endl;
        switch (name_type){
            case 'f':
                strcpy(this->mFileName,name);
                break;
            case 'p':
                strcpy(this->mPackageName,name);
                break;
            default:
                mexErrMsgTxt("Second input should name type.");
        }
        
    }
    
    // setFileName
    void setFileName(const char filename[100]){
        // 'f': for vpz file name
        setName(filename,'f');
    };
    
    // setPackageName
    void setPackageName(const char packagename[100]){
        // 'p': for package name
        setName(packagename,'p');
    };
    
    // getName
    void getName(char name[100],const char name_type){
        switch (name_type){
            case 'f':
                strcpy(name, this->mFileName);
                break;
            case 'p':
                strcpy(name, this->mPackageName);
                break;
            default:
                mexErrMsgTxt("Second input should be a name type ('f', 'p')");
        }
        
    }
    
    // getFileName
    void getFileName(char name[100]){
        // 'f': for vpz file name
        getName(name,'f');
    };
    
    // getPackageName
    void getPackageName(char* name){
        // 'p': for package name
        getName(name,'p');
    };
    
    // init
    void init(){
        // init of manager
        if (!thread_init) {
            manager::init();
            thread_init = true;
        }
        this->mInitStatus=thread_init;
        // Setting vle home dir (from VLE_HOME env var)
        initHomeDir();
    }
    
    // getInitStatus
    void getInitStatus(bool *status){
        status[0]=thread_init;
    }
    
    // getExperimentName
	void getExperimentName(char* expe_name){
        //
        std::string expname(getVpz()->project().experiment().name());
        strcpy(expe_name,(char*)expname.c_str());
    }
    
    // getHomeDir
	void getHomeDir(char *home_dir){
        //
        strcpy(home_dir, this->mHomeDir);
    }
    
    // initHomeDir
    void initHomeDir(){
        // Setting vle home dir (VLE_HOME environment variable)
        mHomeDir=getenv("VLE_HOME");
    }
    
    // openFile
    void openFile()
    {
        assert(this->mPackageName);
        assert(this->mFileName);
        vpz::Vpz*  tmpVpz = 0;
        try {
            //
            Package::package().select(mPackageName);
            std::string filepath = Path::path().getPackageExpFile(mFileName);
            //
            tmpVpz = new vpz::Vpz(filepath);
        } catch(const std::exception& e) {
            //
            tmpVpz=NULL;
        }
        this->mVpz=tmpVpz;
    }
    
    // save
    void save(char* filename){
		getVpz()->write(filename);
		
	}
    
    // getBegin
    void getBegin(double *begin){
        //
        begin[0]=getVpz()->project().experiment().begin();
    }
    
    // setBegin
    void setBegin(double begin){
        //
        getVpz()->project().experiment().setBegin(begin);
    }
    
    // getDuration
    void getDuration(double *duration){
        //
        duration[0]=getVpz()->project().experiment().duration();
    }
    
    // setDuration
    void setDuration(double duration){
        //
        getVpz()->project().experiment().setDuration(duration);
    }
    
    
    // getOutputPlugin
    void getOutputPlugin(char* out_view,char* plugin_name)
    {
        // TO BE FIXED : frozen matlab
        //if (getVpz()->project().experiment().views().outputs().exist(out_view)) {
		if (false){
			vpz::Output& out(getVpz()->project().experiment().views().outputs().get(out_view));
			strcpy(plugin_name,(char*)out.plugin().c_str());
		}else{
			std::string str("");
			strcpy(plugin_name,(char*)str.c_str());
		}
    }
    
    
    // setOutputPlugin with location arg.
    void setOutputPlugin(char* out_view,char* out_location,char* out_destination, char* out_type)
    {
		// mexPrintf("In setOutputPlugin...");
        assert(out_view);
        assert(out_location);
        assert(out_destination);
        assert(out_type);
        
        // adding output stream if doesn't exist
        if (!getVpz()->project().experiment().views().outputs().exist(out_view)){
			vpz::Views& views(getVpz()->project().experiment().views());
			if (strcmp(out_destination, "local") == 0) {
				views.addLocalStreamOutput(out_view,out_location, out_type);
			} else{
				views.addDistantStreamOutput(out_view,out_location,out_type);
			}
		}
        
        vpz::Output& out(getVpz()->project().experiment().views().outputs().get(out_view));
        
        //
        if (strcmp(out_destination, "local") == 0) {
            out.setLocalStream(out_location, out_type);
            // std::cout << "setting local stream " << out.streamformat() << std::endl;
        } else{
            out.setDistantStream(out_location, out_type);
            // std::cout << "setting distant stream " << out.streamformat() << std::endl;
        }
    }
    
    
    
    void addCondition(char *cond_name)
    {
        //Get condition names
        std::string condname(cond_name);
        if (getVpz()->project().experiment().conditions().exist(condname)){
			return;
		}
        vpz::Condition newCond(condname);
        vpz::Conditions& cnd_list(getVpz()->project().experiment().conditions());
        cnd_list.add(newCond);
        //
        
    }
    
    void delCondition(char *cond_name)
    {
        //Get condition names
        std::string condname(cond_name);
        if (getVpz()->project().experiment().conditions().exist(condname)){
			getVpz()->project().experiment().conditions().del(condname);
		}
        //
        
    }
    
    void getConditionsSize(double *size_val)
    {
        //Get condition names
        std::list < std::string > lst;
        getVpz()->project().experiment().conditions().conditionnames(lst);
        //
        size_val[0]=(double)lst.size();
    }
    
    
    void listConditions(mxArray* ret)
    {
        //Get condition names
        std::list < std::string > lst;
        getVpz()->project().experiment().conditions().conditionnames(lst);
        //
        Vle::stringListToMxCellArray(ret,lst);
    }
    
    void getConditionsPortsSize(double *size_val,char *cond_name)
    {
        std::string msgcondname(cond_name);
        const vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(msgcondname));
        size_val[0] = (double)cnd.conditionvalues().size();
        
    }
    //void listConditionPorts
    void listConditionPorts(mxArray* ret,char *cond_name)
    {
        
        //Get condition ports names
        std::string msgcondname(cond_name);
        const vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(msgcondname));
        double size=(double)cnd.conditionvalues().size();
        //iterate on conditions names
        if (size) {
            std::list < std::string > lst;
            cnd.portnames(lst);
            Vle::stringListToMxCellArray(ret,lst);
        }
        else{
            // mexPrintf("\n String list is empty");
        }
    }
    
    // clearConditionPort
    void clearConditionPort(double *ret, const char *cond_name,const char *port_name)
    {
        assert(cond_name);
        assert(port_name);
        
        //Clear port
        *ret = -1;
        try {
            vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(cond_name));
            cnd.clearValueOfPort(port_name);
        } catch(const std::exception& e) {
            *ret = 0;
            mexPrintf("\n matlabvle : ERROR while clearing condition port, message : \n %s \n",e.what());
        }
    }
    
    // getConditionPortValuesSize
    void getConditionPortValuesSize(double *size_val, const char *cond_name,const char *port_name)
    {
        
        assert(cond_name);
        assert(port_name);
        vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(cond_name));
        vle::value::VectorValue& v(cnd.getSetValues(port_name).value());
        // std::cout << "getConditionPortValuesSize::size " << (double)v.size() << std::endl;
        *size_val = (double)v.size();
    }
    
    // getConditionPortValues
    void getConditionPortValues(mxArray *ret, const char *cond_name,const char *port_name)
    {
        
        assert(cond_name);
        assert(port_name);
        
        std::string conditionname_str(cond_name);
        std::string portname_str(port_name);
        
        try {
            vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(conditionname_str));
            const vle::value::VectorValue& vals(vle::value::VectorValue(cnd.getSetValues(portname_str).value()));
            
            int n = 0;
            for (vle::value::VectorValue::const_iterator it = vals.begin();it != vals.end();++it) {
                if (*it) {
                    switch ((*it)->getType()) {
                        case vle::value::Value::BOOLEAN:
                            mxSetCell(ret,n,mxCreateDoubleScalar((double)vle::value::toBoolean(*it)));
                            break;
                        case vle::value::Value::DOUBLE:
                            mxSetCell(ret,n,mxCreateDoubleScalar(vle::value::toDouble(*it)));
                            break;
                        case vle::value::Value::STRING:
                            mxSetCell(ret,n,mxCreateString(vle::value::toString(*it).c_str()));
                            break;
                        case vle::value::Value::INTEGER:
                            mxSetCell(ret,n,mxCreateDoubleScalar((double)vle::value::toInteger(*it)));
                            break;
                        default:
                            mxSetCell(ret,n,mxCreateDoubleScalar(mxGetNaN()));
                            break;
                    }
                } else {
                    mxSetCell(ret,n,mxCreateDoubleScalar(mxGetNaN()));
                }
                
                n++;
            }
        } catch (const std::exception& e) {
            mexPrintf(" matlabvle : ERROR in condition port value : condition-port pair not found : %s - %s \n",conditionname_str.c_str(),portname_str.c_str());
            ret=NULL;
        }
    }
    
    
    
    // getConditionPortValue
    // Functionnality implemented in mex function
    //
    
    // getConditionValueType
    void getConditionValueType(char value_type[30], const char *cond_name,const char *port_name,int idx)
    { 
        assert(conditionname);
        assert(portname);
        
        std::string conditionname_str(cond_name);
        std::string portname_str(port_name);
        
        try {
            vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(conditionname_str));
            const vle::value::VectorValue& vals(vle::value::VectorValue(cnd.getSetValues(portname_str).value()));
			switch (vals[idx]->getType()) {
				case vle::value::Value::BOOLEAN:
					strcpy(value_type,(char*)"boolean");
                    break;
                case vle::value::Value::DOUBLE:
                    strcpy(value_type,(char*)"double");
                    break;
                case vle::value::Value::STRING:
                    strcpy(value_type,(char*)"string");
                    break;
                case vle::value::Value::INTEGER:
                    strcpy(value_type,(char*)"integer");
                    break;
                default:
                    strcpy(value_type,(char*)"none");
                    break;
            }
        } catch (const std::exception& e) {
            mexPrintf(" matlabvle : ERROR in condition port value : condition-port pair not found : %s - %s \n",conditionname_str.c_str(),portname_str.c_str());
            value_type=(char*)"none";
        }
    }
    
    // addRealCondition
    void addRealCondition(double *ret,const char* conditionname, const char* portname, double value)
    {
        assert(conditionname);
        assert(portname);
        
        std::string conditionname_str(conditionname);
        std::string portname_str(portname);
        //
        *ret = -1;
        try {
            //
            vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(conditionname_str));
            cnd.addValueToPort(portname_str, vle::value::Double::create(value));
            
        } catch(const std::exception& e) {
            *ret=0;
        }
    }
    
    // addIntegerCondition
    void addIntegerCondition(double *ret,const char* conditionname, const char* portname, long value)
    {
        assert(conditionname);
        assert(portname);
        
        std::string conditionname_str(conditionname);
        std::string portname_str(portname);
        //
        *ret = -1;
        try {
            //
            vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(conditionname_str));
            cnd.addValueToPort(portname_str, vle::value::Integer::create(value));
            
        } catch(const std::exception& e) {
            *ret=0;
        }
    }
    
    // addBooleanCondition(condition_name,port_name,value_content)
    void addBooleanCondition(double *ret,const char* conditionname, const char* portname, bool value)
    {
        assert(conditionname);
        assert(portname);
        
        std::string conditionname_str(conditionname);
        std::string portname_str(portname);
        //
        *ret = -1;
        try {
            //
            vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(conditionname_str));
            cnd.addValueToPort(portname_str, vle::value::Boolean::create(value));
            
        } catch(const std::exception& e) {
            *ret=0;
        }
    }
    // addStringCondition
    void addStringCondition(double *ret,const char* conditionname, const char* portname, const char* value)
    {
        assert(conditionname);
        assert(portname);
        
        std::string conditionname_str(conditionname);
        std::string portname_str(portname);
        //
        *ret = -1;
        try {
            vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(conditionname_str));
            cnd.addValueToPort(portname_str, vle::value::String::create(value));
            
        } catch(const std::exception& e) {
            *ret=0;
        }
    }
    
    // setConditionValue
    void setConditionValue(double *ret,const char* conditionname,const char* portname, const char* value,const char* valuetype,int valueposition)
    {
        assert(conditionname);
        assert(portname);
        
        std::string conditionname_str(conditionname);
        std::string portname_str(portname);
        int i=valueposition;
        //
        *ret = -1;
        try {
            vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(conditionname_str));
            
            vle::value::VectorValue& vector(cnd.getSetValues(portname_str).value());
            
            if (strcmp(valuetype, (char*)"integer") == 0) {
                vector[i] = value::Integer::create(boost::lexical_cast < int > (value));
            }else if (strcmp(valuetype, (char*)"double") == 0)      {
                vector[i] = value::Double::create(boost::lexical_cast < double > (value));
            }else if (strcmp(valuetype, (char*)"string") == 0)      {
                vector[i] = value::String::create(value);
            }else if (strcmp(valuetype, (char*)"boolean") == 0)      {
                bool val;
                if (strcmp(valuetype, (char*)"true") == 0) {
                    val = true;
                }else {
                    val = false;
                }
                vector[i] = value::Boolean::create(val);
            }else  {
                vector[i] = value::String::create(value);
            }
        }       catch(const std::exception& e) {
            *ret=0;
        }
    }
    
    // getViewOutput
    void getViewOutput(char *view_name)
    {
        //
        vpz::View& view(getVpz()->project().experiment().views().get(view_name));
        strcpy(view_name,(char*)view.output().c_str());
    }
    
    
    // addView
    // All view types
    // timed view with default time step
    void addView(char *view_name,char *view_type,char *output)
    {	
		if (strcmp(view_type, (char*)"timed") == 0) {
			// default time step !
			double time=1;
			getVpz()->project().experiment().views().addTimedView(view_name, time, output);
		}else if (strcmp(view_type, (char*)"event") == 0)      {
			getVpz()->project().experiment().views().addEventView(view_name, output);
		}else if (strcmp(view_type, (char*)"finish") == 0)      {
			getVpz()->project().experiment().views().addFinishView(view_name, output);
		}else{
			mexErrMsgTxt("unknown view type !");
		}
	}
    
    // addView
    // for specific time step
    void addView(char *view_name,char *view_type,char *output,double *time)
    {	
		if (strcmp(view_type, (char*)"timed") == 0) {
			getVpz()->project().experiment().views().addTimedView(view_name, *time, output);
		}else{
			mexErrMsgTxt("Only for timed view !");	
		}
    }
    
    // TODO: delView
    
    
    // listViews
    void listViews(mxArray* ret)
    {
        //Get view names
        std::list < std::string > lst;
        vpz::ViewList& viewlst(getVpz()->project().experiment().views().viewlist());
        //
        int size = viewlst.size();
        
        vpz::ViewList::iterator it = viewlst.begin();
        while (it != viewlst.end()) {
                lst.push_back(it->first);
                ++it;
        }
        
        Vle::stringListToMxCellArray(ret,lst);
    }
    
    // getViewsSize
    void getViewsSize(double *size_val)
   {
		// size
		size_val[0]=(double)getVpz()->project().experiment().views().viewlist().size();
   }
    
    
    // existView(view_name)
    void existView(double *exist_view,char *view_name)
    {
        //
        if (getVpz()->project().experiment().views().exist(view_name)){
            *exist_view=1;
        } else {
            *exist_view=0;
        }
    }
    
    //
    
    // existObservable(obs_name)
    void existObservable(double *exist_obs,char *obs_name)
    {
        //
        if (getVpz()->project().experiment().views().observables().exist(obs_name)){
            *exist_obs=1;
        } else {
            *exist_obs=0;
        }
    }
    
    
    // listObservables
    void listObservables(mxArray* ret)
   {
		//Get observable names list
		vpz::ObservableList& obslst(getVpz()->project().experiment().views().observables().observablelist());
		// list to cell
        Vle::stringListToMxCellArray(ret,obslst);
   }
    
    
    // getObservablesSize
    void getObservablesSize(double *size_val)
   {
		//Get observable names list
		vpz::ObservableList& obslst(getVpz()->project().experiment().views().observables().observablelist());
		// size
		size_val[0]=(double)obslst.size();
   }
    
    // clearObservables()
    void clearObservables()
   {
		//Get observable names list
		getVpz()->project().experiment().views().observables().clear();
   }
    
    // addObservable(obs_name)
     void addObservable(double *ret,char* obsname)
   {
	   std::string obsname_str(obsname);
		//add observable to list
		
		*ret = -1;
        try {
            //
            getVpz()->project().experiment().views().observables().add(obsname_str);
            
        } catch(const std::exception& e) {
            *ret=0;
        }
		
   }
    
    // delObservable(obs_name)
    void delObservable(char* obsname)
   {
	   std::string obsname_str(obsname);
		//del observable into list
		getVpz()->project().experiment().views().observables().del(obsname_str);
   }
    
    
    // getObservablePortsSize(obs_name)
    void getObservablePortsSize(double *size_val,char *obsname)
    {
        std::string obs_name(obsname);
        vpz::ObservablePortList& obsportlst(getVpz()->project().experiment().views().observables().get(obs_name).observableportlist());
        size_val[0] = (double)obsportlst.size();
        
    }
    
    // listObservablePorts(obs_name)
    void listObservablesPorts(mxArray* ret,char *obsname)
    {
        std::string obs_name(obsname);
        vpz::ObservablePortList& obsportlst(getVpz()->project().experiment().views().observables().get(obs_name).observableportlist());
        double size = (double)obsportlst.size();
        
        if (size) {
            Vle::stringListToMxCellArray(ret,obsportlst);
        }
        else{
            // mexPrintf("\n String list is empty");
        }
    }
    
    // addObservablePort(obs_name,port_name)
    void addObservablePort(char *obsname,char *portname)
    {
		std::string obs_name(obsname);
        std::string port_name(portname);
        
		if (getVpz()->project().experiment().views().observables().exist(obs_name)){
			vpz::Observable& obs(getVpz()->project().experiment().views().observables().get(obs_name));
			if (!obs.exist(port_name)){
				obs.add(port_name);
			}
		}
    }
    
    // delObservablePort(obs_name,port_name)
    void delObservablePort(char *obsname,char *portname)
    {
        std::string obs_name(obsname);
        std::string port_name(portname);
        
        if (getVpz()->project().experiment().views().observables().exist(obs_name)){
			vpz::Observable& obs(getVpz()->project().experiment().views().observables().get(obs_name));
			if (obs.exist(port_name)){
				obs.del(port_name);
			}
		}
    }
    

    // addViewToObservablePort(obs_name,port_name,view_name)
    void addViewToObservablePort(char *obsname,char *portname,char *viewname){
		std::string obs_name(obsname);
        std::string port_name(portname);
        std::string view_name(viewname);
        // test if obs and port exist
		vpz::ObservablePort& obsport(getVpz()->project().experiment().views().observables().get(obs_name).get(port_name));
		if ((getVpz()->project().experiment().views().exist(view_name)) && (!obsport.exist(view_name))) {
			obsport.add(view_name);
		}
	
	}
    // delViewFromObservablePort(obs_name,port_name,view_name)
    void delViewFromObservablePort(char *obsname,char *portname,char *viewname){
		std::string obs_name(obsname);
        std::string port_name(portname);
        std::string view_name(viewname);
        // test if obs and port exist
		vpz::ObservablePort& obsport(getVpz()->project().experiment().views().observables().get(obs_name).get(port_name));
		if ((getVpz()->project().experiment().views().exist(view_name)) && (obsport.exist(view_name))) {
			obsport.del(view_name);
		}
	
	}
    
    // existObservablePortView(obs_name,port_name,view_name)
    void existObservablePortView(double *exist_obs,char *obsname,char *portname,char *viewname){
		std::string obs_name(obsname);
        std::string port_name(portname);
        std::string view_name(viewname);
		vpz::ObservablePort& obsport(getVpz()->project().experiment().views().observables().get(obs_name).get(port_name));
		if ((getVpz()->project().experiment().views().exist(view_name)) && (obsport.exist(view_name))) {
			*exist_obs=1;
		} else {
			*exist_obs=0;
		}
	
	}
    
    
    // getDynamicsSize
    void getDynamicsSize(double *size_val)
    {
        vpz::DynamicList& lst(getVpz()->project().dynamics().dynamiclist());
        size_val[0]=(double)lst.size();
    }
    
    // listDynamics
    void listDynamics(mxArray* ret)
    {
        vpz::DynamicList& lst(getVpz()->project().dynamics().dynamiclist());
        //getting dynamics names
        Vle::stringListToMxCellArray(ret,lst);
    }
    
    // getDynamicModelsSize
    void getDynamicModelsSize(double *size_val,char *dyn_name)
    {
        vpz::AtomicModelList& atommods(getVpz()->project().model().atomicModels());
        vpz::AtomicModelList::iterator it = atommods.begin();
        std::string msgdynname(dyn_name);
        int i=0;
        while (it != atommods.end()) {
            if (it->second.dynamics() == msgdynname) {
                ++i;
            }
            ++it;
        }
        size_val[0]=(double)i;
    }
    
    // listDynamicModels
    void listDynamicModels(mxArray* ret,char *dyn_name)
    {
        vpz::AtomicModelList& atommods(getVpz()->project().model().atomicModels());
        vpz::AtomicModelList::iterator it = atommods.begin();
        std::string msgdynname(dyn_name);
        int i=0;
        std::list < std::string > lst;
        while (it != atommods.end()) {
            if (it->second.dynamics() == msgdynname) {
                std::string atomname = it->first->getName();
                lst.push_back(atomname);
                ++i;
            }
            ++it;
        }
        if (i>0) {
            Vle::stringListToMxCellArray(ret,lst);
        }
        else{
            // mexPrintf("\n String list is empty");
        }
    }
    
    // getAtomicModelId
    void getAtomicModelId(char *model_name,double *model_id){
		vpz::AtomicModelList& atommods(getVpz()->project().model().atomicModels());
        vpz::AtomicModelList::iterator it = atommods.begin();
        std::string msgmodname(model_name);
		int i=0;
        while (it != atommods.end()) {
            if (it->first->getName() == msgmodname) {
                break;
            }
            ++i;
            ++it;
        }
        model_id[0]=(double)i;
	}
    
    // getAtomicModel
    vpz::AtomicModel& getAtomicModel(char *model_name){
		vpz::AtomicModelList& atommods(getVpz()->project().model().atomicModels());
        vpz::AtomicModelList::iterator it = atommods.begin();
        std::string msgmodname(model_name);
		//
        while (it != atommods.end()) {
            if (it->first->getName() == msgmodname) {
                break;
            }
            ++it;
        }
        return it->second;
	}
	
	// getModelObservables
    void getModelObservables(char *modelname,char *obsname){
		vpz::AtomicModel& a=getAtomicModel(modelname);
		strcpy(obsname,(char*)a.observables().c_str());
	}
    
    // setModelObservables
    void setModelObservables(char *modelname,char *obsname){
		vpz::AtomicModel& a=getAtomicModel(modelname);
		a.setObservables(obsname);
	}
	
	
	 // getGraphModel
    vle::graph::Model* getGraphModel(char *model_name){
		vpz::AtomicModelList& atommods(getVpz()->project().model().atomicModels());
        vpz::AtomicModelList::iterator it = atommods.begin();
        std::string msgmodname(model_name);
		
        while (it != atommods.end()) {
            if (it->first->getName() == msgmodname) {
                break;
            }
            ++it;
        }
        return it->first;
	}
    
    // existGraphModel
    bool existGraphModel(char *model_name){
		vpz::AtomicModelList& atommods(getVpz()->project().model().atomicModels());
        vpz::AtomicModelList::iterator it = atommods.begin();
        std::string msgmodname(model_name);
		bool ex=false;
        while (it != atommods.end()) {
            if (it->first->getName() == msgmodname) {
                ex=true;
                break;
            }
            ++it;
        }
        return ex;
	}
    
    
    // getOutputPortNumber
    void getOutputPortNumber(char *modelname,double *outport_num){
		vle::graph::Model *g=getGraphModel(modelname);
		outport_num[0]=(double)g->getOutputPortNumber();
	}
	
	// existOutputPort
	void existOutputPort(char *modelname,char *portname, double *port_exists){
		vle::graph::Model *g=getGraphModel(modelname);
		std::string msgportname(portname);
		port_exists[0]=(double)g->existOutputPort(msgportname);
		
	}
	
	
	// listOutputPorts
	void listOutputPorts(mxArray* ret,char *modelname)
    {
		vle::graph::Model *g=getGraphModel(modelname);
		
		std::string mod=g->getName();
			
		vle::graph::ConnectionList& connlist=g->getOutputPortList();
	        
        vle::graph::ConnectionList::iterator it = connlist.begin();
        
        int i=0;
        std::list < std::string > lst;
        while (it != connlist.end()) {
            lst.push_back(it->first.c_str());
            ++i;
            ++it;
        }
        if (i>0){
            Vle::stringListToMxCellArray(ret,lst);
        }else{
            // mexPrintf("\n String list is empty");
        }
        
        
    }
	
	// addOutputPort
	void addOutputPort(char *modelname,char *portname){
		vle::graph::Model *g=getGraphModel(modelname);
		std::string msgportname(portname);
		vle::graph::ModelPortList& portlist=g->addOutputPort(msgportname);
		
	}
    
    // delOutputPort
    void delOutputPort(char *modelname,char *portname){
		vle::graph::Model *g=getGraphModel(modelname);
		std::string msgportname(portname);
		g->delOutputPort(msgportname);
	}
    
    
    // run for storage plugin
    void run(mxArray* ret ,const char *return_type)
    {
        assert(return_type);
        try {
            // Simulate 1 simulation
            vle::manager::RunQuiet jrm;
            jrm.start(*getVpz());
            
            const vle::oov::OutputMatrixViewList& result1(jrm.outputs());
            // converting results according to views list for storage plugin
            outputMatrixViewListToMxCellArray(ret,result1,return_type);
            
        } catch(const std::exception& e) {
            mexPrintf("\n matlabvle : ERROR while running simulation, message : \n %s \n",e.what());
            ret=NULL;
        }
    }
    
    // run for file plugin
    void run()
    {
        try {
            // 1 simulation
            vle::manager::RunQuiet jrm;
            jrm.start(*getVpz());
        } catch(const std::exception& e) {
            mexPrintf("\n matlabvle : ERROR while running simulation, message : \n %s \n",e.what());
            
        }
    }
    
    vpz::Vpz* getVpz()
    {
        //Cast of inputs
        assert(this->mVpz);
        vpz::Vpz*  vpz(reinterpret_cast < vpz::Vpz* >(this->mVpz));
        assert(vpz);
        return vpz;
    }
    
    
    static void stringListToMxCellArray(mxArray* ret,std::list < std::string > lst)
    {
        // to be added : dimensions of CellMatrix ret // lst.size()
        if (lst.size()){
            std::list < std::string >::iterator it = lst.begin();
            for (unsigned int i=0; i < lst.size(); ++i) {
                mxSetCell(ret,i,mxCreateString((*it).c_str()));
                it++;
            }
        }
        else{
            // mexPrintf("\n std::string -> String list is empty");
        }
    }
    
    static void stringListToMxCellArray(mxArray* ret,vpz::DynamicList& dynlist)
    {
        // to be added : dimensions of CellMatrix ret // dynlist.size()
        int i=0;
        if (dynlist.size()){
            vpz::DynamicList::iterator dynit;
            for (dynit = dynlist.begin(); dynit != dynlist.end(); ++dynit, ++i) {
                mxSetCell(ret,i,mxCreateString(dynit->first.c_str()));
            }
        }
        else{
            // mexPrintf("\n  vpz::DynamicList -> String list is empty");
        }
    }
    
    static void stringListToMxCellArray(mxArray* ret,vpz::ObservableList& obslist)
    {
        // to be added : dimensions of CellMatrix ret // obslist.size()
        int i=0;
        if (obslist.size()){
            vpz::ObservableList::iterator opit;
            for (opit = obslist.begin(); opit != obslist.end(); ++opit, ++i) {
                mxSetCell(ret,i,mxCreateString(opit->first.c_str()));
            }
        }
        else{
            // mexPrintf("\n vpz::ObservableList -> String list is empty");
        }
    }
    
    static void stringListToMxCellArray(mxArray* ret,vpz::ObservablePortList& obsportlist)
    {
        // to be added : dimensions of CellMatrix ret // obsportlist.size()
        int i=0;
        if (obsportlist.size()){
            vpz::ObservablePortList::iterator opit;
            for (opit = obsportlist.begin(); opit != obsportlist.end(); ++opit, ++i) {
                mxSetCell(ret,i,mxCreateString(opit->first.c_str()));
            }
        }
        else{
            // mexPrintf("\n vpz::ObservablePortList -> String list is empty");
        }
    }
    
	/*void getMethodsList(mxArray* ret){
	}*/
	
	void setMethodsList(){
			// For filling methods list usable through mex function ...  
		
			std::string mlist[]={"getFileName" , "setFileName" , "getPackageName" , "setPackageName", \
            "openFile" , "save" , "getStatus" , "getHomeDir" , \
            "getExperimentName" , "getBegin" , "setBegin" , "getDuration" , "setDuration" , \
            "setOutputPlugin" , "getOutputPlugin" , \
            "addCondition" , "delCondition" , "getConditionsSize" , "listConditions" , "getConditionPortsSize", \
            "listConditionPorts" , "clearConditionPort" , "getConditionPortValuesSize", \
            "getConditionPortValues" , "getConditionPortValue" , "getConditionValueType" , \
            "addRealCondition" , "addIntegerCondition" , "addStringCondition" , "addBooleanCondition", \
            "getViewOutput" , "getViewsSize" , "listViews" , "existView" , "addView" , \
            "getDynamicsSize" , "listDynamics" , "getDynamicModelsSize" , "listDynamicModels" , "setConditionValue" , \
            "existObservable" , "listObservables" , "getObservablesSize" , "clearObservables" , \
            "addObservable" , "delObservable" , \
            "listObservablePorts" , "getObservablePortsSize" , "addObservablePort" , "delObservablePort" , \
            "existObservablePortView" , "addViewToObservablePort" , "delViewFromObservablePort" , \
            "getAtomicModelId" , "getModelObservables" , "setModelObservables", \ 
            "getOutputPortNumber" , "existOutputPort" , "addOutputPort" , "delOutputPort" , "listOutputPorts", \
            "run" , \
            "existMethod"};
            
            int sz=sizeof(mlist)/sizeof(mlist[0]);
          
        for (int i=0; i<=sz; i++){ 
			this->mMethodsList.push_back(((char*)mlist[i].c_str()));
        }
	}
	
	
	void existMethod(double *method_exists,char *method_name){
		// Telling if a method is available for the matlab Vle interface class
		std::string method(method_name);
		*method_exists=1;
		std::list<std::string>::iterator findIter = std::find(this->mMethodsList.begin(), this->mMethodsList.end(), method_name);
		if (findIter == this->mMethodsList.end()){
			*method_exists=0;
		}	
	}
	
	
private:
    matlabvle_t mVpz;
    char mFileName[100];
    char mPackageName[100];
    bool mInitStatus;
    char *mHomeDir;
    std::list<std::string> mMethodsList;
};




void checkInputArgs(char *func_name,int in_args_nb,int in_needed_nb)
{
	// checking input argument number passed through the mex function/function name
	// Throwing an exception if not enough numerous.
	// No error message Id is given in exception message.
	std::string msg(func_name);
	std::ostringstream nargs;
	nargs << in_needed_nb-2; // effective number given by user, in_needed_nb==total for mex function (in and out)
	msg.append(" : missing argument(s), ");
	msg.append(nargs.str());
	msg.append(" needed !");

	if (in_args_nb < in_needed_nb)
            mexErrMsgIdAndTxt("MATLABVLE:narginchk:notEnoughInputs",msg.c_str());
}


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    // Getting the command string
    char cmd[64];
    if (nrhs < 1 || mxGetString(prhs[0], cmd, sizeof(cmd)))
        mexErrMsgTxt("First input should be a command string less than 64 characters long.");
    
    
    // new, for instanciation
    if (!strcmp("new", cmd)) {
        // Check parameters
        if(nlhs != 1)
            mexErrMsgTxt("New: One output expected.");
        // Returns a handle to a new C++ instance
        switch (nrhs){
            case 1:
                plhs[0] = convertPtr2Mat<Vle>(new Vle);
                break;
            case 3:
                char* pkgname=mxArrayToString(prhs[1]);
                char* filename=mxArrayToString(prhs[2]);
                plhs[0] = convertPtr2Mat<Vle>(new Vle(pkgname,filename));
                break;
                //default:
                // TODO : define action : raise an exception ?
        }
        return;
    }
    
    // Check if there is a second input, this should be the class instance handle
    if (nrhs < 2)
        mexErrMsgTxt("Second input should be a class instance handle.");
    
    // delete object
    if (!strcmp("delete", cmd)) {
        // Destroy the C++ object
        destroyObject<Vle>(prhs[1]);
        // Warn if other commands were ignored
        if (nlhs != 0 || nrhs != 2)
            mexWarnMsgTxt("Delete: Unexpected arguments ignored.");
        return;
    }
    
    // Get the class instance pointer from the second input
    Vle *vle_instance = convertMat2Ptr<Vle>(prhs[1]);
    
    
    ///////////////////////////////////////////////////////////
    // Calling the class methods
    ///////////////////////////////////////////////////////////
    
    
    // setFileName(in_name)
    if (!strcmp("setFileName", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        
        char *in_name=mxArrayToString(prhs[2]);
        
        vle_instance->setFileName(in_name);
        return;
    }
    
    
    // getFileName()
    if (!strcmp("getFileName", cmd)) {
        char out_name[100];
        vle_instance->getFileName(out_name);
        plhs[0] = mxCreateString(out_name);
        return;
    }
    
    
    // setPackageName(in_name)
    if (!strcmp("setPackageName", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
    
        char* in_name=mxArrayToString(prhs[2]);
        vle_instance->setPackageName(in_name);
        return;
    }
    
    
    // getPackageName()
    if (!strcmp("getPackageName", cmd)) {
        char out_name[100];
        vle_instance->getPackageName(out_name);
        plhs[0] = mxCreateString(out_name);
        return;
    }
    
    
    // openFile()
    if (!strcmp("openFile", cmd)) {
        vle_instance->openFile();
        return;
    }
    
    
    // save(file_name)
    if (!strcmp("save", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
		
		char* file_name=mxArrayToString(prhs[2]);

        vle_instance->save(file_name);
        return;
    }
    
    
    // getInitStatus()
    if (!strcmp("getInitStatus", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,2);
        
        bool *out_status;
        vle_instance->getInitStatus(out_status);
        plhs[0]=mxCreateLogicalScalar(*out_status);
        return;
    }
    
    
    // getHomeDir()
    if (!strcmp("getHomeDir", cmd)) {
 
        char home_dir[100];
        vle_instance->getHomeDir(home_dir);
        plhs[0] = mxCreateString(home_dir);
        return;
    }
    
    
    // getExperimentName()
    if (!strcmp("getExperimentName", cmd)) {
        char expe_name[100];
        vle_instance->getExperimentName(expe_name);
        plhs[0] = mxCreateString(expe_name);
        return;
    }
    
    
    // getBegin()
    if (!strcmp("getBegin", cmd)) {   
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_begin=mxGetPr(plhs[0]);
        vle_instance->getBegin(out_begin);
        return;
    }
    
    
    // setBegin(in_begin)
    if (!strcmp("setBegin", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        double in_begin=mxGetScalar(prhs[2]);
        vle_instance->setBegin(in_begin);
        return;
    }
    
    
    // getDuration()
    if (!strcmp("getDuration", cmd)) {
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_duration=mxGetPr(plhs[0]);
        vle_instance->getDuration(out_duration);
        return;
    }
    
    
    // setDuration(in_duration)
    if (!strcmp("setDuration", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        double in_duration=mxGetScalar(prhs[2]);
        vle_instance->setDuration(in_duration);
        return;
    }
    
    
    // getOutputPlugin(view_name)
    if (!strcmp("getOutputPlugin", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        char* plugin_name=mxArrayToString(mxCreateString(""));
        //char plugin_name[100];
        char* out_view=mxArrayToString(prhs[2]);
        vle_instance->getOutputPlugin(out_view,plugin_name);
        plhs[0] = mxCreateString(plugin_name);
        return;
    }
    
    // setOutputPlugin(out_view,out_location,out_destination,out_type)
    if (!strcmp("setOutputPlugin", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,6);
        char* out_view=mxArrayToString(prhs[2]);
        char* out_location=mxArrayToString(prhs[3]);
        char* out_destination=mxArrayToString(prhs[4]);
        char* out_type=mxArrayToString(prhs[5]);
        vle_instance->setOutputPlugin(out_view,out_location,out_destination,out_type);
        return;
    }
    
    
    // addCondition(cond_name)
    if (!strcmp("addCondition", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        char* cond_name=mxArrayToString(prhs[2]);
        vle_instance->addCondition(cond_name);
        return;
    }
    
    // delCondition(cond_name)
    if (!strcmp("delCondition", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        char* cond_name=mxArrayToString(prhs[2]);
        vle_instance->delCondition(cond_name);
        return;
    }
    
    // getConditionsSize()
    if (!strcmp("getConditionsSize", cmd)) {
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_size=mxGetPr(plhs[0]);
        vle_instance->getConditionsSize(out_size);
        return;
    }
    
    
    // listConditions()
    if (!strcmp("listConditions", cmd)) {
        // get conditions list size
        double *max_cond=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        vle_instance->getConditionsSize(max_cond);
        plhs[0] = mxCreateCellMatrix(1,(int)*max_cond);
        vle_instance->listConditions(plhs[0]);
        
        return;
    }
    
    
    // getConditionPortsSize(cond_name)
    if (!strcmp("getConditionPortsSize", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,3);
        char *cond_name=mxArrayToString(prhs[2]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_size=mxGetPr(plhs[0]);
        vle_instance->getConditionsPortsSize(out_size,cond_name);
        return;
    }
    
    
    // listConditionPorts(cond_name)
    if (!strcmp("listConditionPorts", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        char *cond_name=mxArrayToString(prhs[2]);
        double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        vle_instance->getConditionsPortsSize(out_size,cond_name);
        plhs[0] = mxCreateCellMatrix(1,(int)*out_size);
        vle_instance->listConditionPorts(plhs[0],cond_name);
        return;
    }
    
    
    // clearConditionPort(condition_name,port_name)
    if (!strcmp("clearConditionPort", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,4);
        
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->clearConditionPort(status,cond_name,port_name);
        return;
    }
    
    
    // getConditionPortValuesSize(cond_name,port_name)
    if (!strcmp("getConditionPortValuesSize", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,4);
        
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        
        
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_size=mxGetPr(plhs[0]);
        vle_instance->getConditionPortValuesSize(out_size,cond_name,port_name);
        return;
    }
    
    
    // getConditionPortValues(cond_name,port_name)
    if (!strcmp("getConditionPortValues", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,4);
        
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        
        vle_instance->getConditionPortValuesSize(out_size,cond_name,port_name);
        
        plhs[0]=mxCreateCellMatrix((mwSize)1,(mwSize)(int)*out_size);
        
        vle_instance->getConditionPortValues(plhs[0],cond_name,port_name);
        return;
    }
    
    
    // getConditionPortValue(cond_name,port_name,value_pos)
    if (!strcmp("getConditionPortValue", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,5);
        
        char value_type[30];
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        double value_pos=mxGetScalar(prhs[4]);
        double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        mxArray *port_values;
        
        vle_instance->getConditionPortValuesSize(out_size,cond_name,port_name);
        // testing Values dimension
        if(value_pos >= *out_size)
			mexErrMsgTxt("Value position is out of Values dimensions ! "); 
		
		
        vle_instance->getConditionValueType(value_type,cond_name,port_name,(int)value_pos);
        
        port_values=mxCreateCellMatrix((mwSize)1,(mwSize)(int)*out_size);;
        vle_instance->getConditionPortValues(port_values,cond_name,port_name);
        
        mxArray *out;
        
        try{
			if (value_pos<*out_size){
				mxArray *port_value = mxGetCell(port_values,value_pos);
				if (strcmp(value_type, (char*)"string") == 0){
					out=mxCreateString(mxArrayToString(port_value));
				}else if (strcmp(value_type, (char*)"boolean") == 0) {
					out=mxCreateLogicalScalar((bool)mxGetScalar(port_value));
				}else if (strcmp(value_type, (char*)"double") == 0) {
					out=mxCreateDoubleScalar((double)mxGetScalar(port_value));
				}else if (strcmp(value_type, (char*)"integer") == 0) {
					out=mxCreateDoubleScalar((double)mxGetScalar(port_value));
				}else{
					out=mxCreateDoubleScalar(mxGetNaN());
				}
			}
        } catch (const std::exception& e) {
			out=mxCreateDoubleScalar(mxGetNaN());
		}
	    plhs[0]= out;
        return;
    }
    
    
    // getConditionValueType(cond_name,port_name,value_pos)
    if (!strcmp("getConditionValueType", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,5);
        
        char value_type[30];
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        double value_pos=mxGetScalar(prhs[4]);
        vle_instance->getConditionValueType(value_type,cond_name,port_name,(int)value_pos);
        plhs[0] = mxCreateString(value_type);
        return;
    }
    
    
    // addRealCondition(condition_name,port_name,value)
    if (!strcmp("addRealCondition", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,5);
        
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        double value=mxGetScalar(prhs[4]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->addRealCondition(status,cond_name,port_name,value);
        return;
    }
    
    
    // addIntegerCondition(condition_name,port_name,value)
    if (!strcmp("addIntegerCondition", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,5);
        
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        double value=mxGetScalar(prhs[4]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->addRealCondition(status,cond_name,port_name,(int)value);
        return;
    }
    
    
    // addBooleanCondition(condition_name,port_name,value)
    if (!strcmp("addBooleanCondition", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,5);
        
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        double value=mxGetScalar(prhs[4]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->addBooleanCondition(status,cond_name,port_name,(bool)value);
        return;
    }
    
    
    // addStringCondition(condition_name,port_name,value)
    if (!strcmp("addStringCondition", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,5);
        
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        char *value=mxArrayToString(prhs[4]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->addStringCondition(status,cond_name,port_name,value);
        return;
    }
    
    
    // setConditionValue(condition_name,port_name,value,value_type,value_position)
    if (!strcmp("setConditionValue", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,7);
        
        char *cond_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        char *value=mxArrayToString(prhs[4]);
        char *value_type=mxArrayToString(prhs[5]);
        double value_position=mxGetScalar(prhs[6]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->setConditionValue(status,cond_name,port_name,value,value_type,(int)value_position);
        return;
    }
    
    
    // getViewOutput()
    if (!strcmp("getViewOutput", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        
        char *view_name=mxArrayToString(prhs[2]);
        vle_instance->getViewOutput(view_name);
        plhs[0] = mxCreateString(view_name);
        return;
    }
    
    // addView(view_name,view_type,output)
    if (!strcmp("addView", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,5);
		char *view_name=mxArrayToString(prhs[2]);
		char *view_type=mxArrayToString(prhs[3]);
		char *output=mxArrayToString(prhs[4]);
		vle_instance->addView(view_name,view_type,output);
		return;
	}
    
    // TODO : delView()
    
    
    // listViews()
    if (!strcmp("listViews", cmd)) {
        double *views_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        vle_instance->getViewsSize(views_size);
        plhs[0] = mxCreateCellMatrix(1,(int)*views_size);
        vle_instance->listViews(plhs[0]);
        return;
    }
    
    
    // getViewsSize()
    if (!strcmp("getViewsSize", cmd)) {
		
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *views_size=mxGetPr(plhs[0]);
        vle_instance->getViewsSize(views_size);
        return;
    }
    
    
    // existView(view_name)
    if (!strcmp("existView", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        
        char *view_name=mxArrayToString(prhs[2]);
        
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->existView(status,view_name);
        return;
    }
    
    
    // existObservable(obs_name)
    if (!strcmp("existObservable", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        
        char *obs_name=mxArrayToString(prhs[2]);
        
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->existObservable(status,obs_name);
        return;
    }
    
    
    // listObservables()
    if (!strcmp("listObservables", cmd)) {
        
        double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        vle_instance->getObservablesSize(out_size);
	
        plhs[0] = mxCreateCellMatrix(1,(int)*out_size);
        vle_instance->listObservables(plhs[0]);
        return;
    }
    
    
    // getObservablesSize()
    if (!strcmp("getObservablesSize", cmd)) {
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_size=mxGetPr(plhs[0]);
        vle_instance->getObservablesSize(out_size);
        return;
    }
    
    
    // clearObservables()
    if (!strcmp("clearObservables", cmd)) {
        vle_instance->clearObservables();
        return;
    }
    
    
    // addObservable(obs_name)
    if (!strcmp("addObservable", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,3);
        
        char *obs_name=mxArrayToString(prhs[2]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->addObservable(status,obs_name);
        return;
    }
    
    
    // delObservable(obs_name)
    if (!strcmp("delObservable", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,3);
        
        char *obs_name=mxArrayToString(prhs[2]);
        vle_instance->delObservable(obs_name);
        return;
    }
    
    
    // getObservablePortsSize(obs_name)
    if (!strcmp("getObservablePortsSize", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,3);
        
        char *obs_name=mxArrayToString(prhs[2]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_size=mxGetPr(plhs[0]);
        vle_instance->getObservablePortsSize(out_size,obs_name);
        return;
    }
    
    
    // listObservablePorts(obs_name)
    if (!strcmp("listObservablePorts", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,3);
        
        char *obs_name=mxArrayToString(prhs[2]);
        double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        vle_instance->getObservablePortsSize(out_size,obs_name);
        plhs[0] = mxCreateCellMatrix(1,(int)*out_size);
        vle_instance->listObservablesPorts(plhs[0],obs_name);
        return;
    }
    
    
    // addObservablePort(obs_name,port_name)
	if (!strcmp("addObservablePort", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,4);
        
        char *obs_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        
        vle_instance->addObservablePort(obs_name,port_name);
        return;
    }
    
    
    // delObservablePort(obs_name,port_name)
    if (!strcmp("delObservablePort", cmd)) {
		// Checking input parameters
        checkInputArgs(cmd,nrhs,4);
        
        char *obs_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        
        vle_instance->delObservablePort(obs_name,port_name);
        return;
    }
    
    
    // addViewToObservablePort(obs_name,port_name,view_name)
    if (!strcmp("addViewToObservablePort", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,5);
        
        char *obs_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        char *view_name=mxArrayToString(prhs[4]);
        
        vle_instance->addViewToObservablePort(obs_name,port_name,view_name);
        return;
    }
    // SEE addViewToObservablePorts : for multiple ports (cell: mxIsClass)
    
    
    // delViewFromObservablePort(obs_name,port_name,view_name)
    if (!strcmp("delViewFromObservablePort", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,5);
        
        char *obs_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        char *view_name=mxArrayToString(prhs[4]);
        
        vle_instance->delViewFromObservablePort(obs_name,port_name,view_name);
        return;
    }
    // SEE delViewFromObservablePorts : for multiple ports (cell: mxIsClass)
    
    
    // existObservablePortView(obs_name,port_name,view_name)
    if (!strcmp("existObservablePortView", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,5);
        
        char *obs_name=mxArrayToString(prhs[2]);
        char *port_name=mxArrayToString(prhs[3]);
        char *view_name=mxArrayToString(prhs[4]);
        
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->existObservablePortView(status,obs_name,port_name,view_name);
        return;
    }
    
    
    // getDynamicsSize()
    if (!strcmp("getDynamicsSize", cmd)) {

        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_size=mxGetPr(plhs[0]);
        vle_instance->getDynamicsSize(out_size);
        return;
    }
    
    
    // listDynamics()
    if (!strcmp("listDynamics", cmd)) {
        // get conditions list size
        
        double *max_cond=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        vle_instance->getDynamicsSize(max_cond);
        
        plhs[0] = mxCreateCellMatrix(1,(int)*max_cond);
        vle_instance->listDynamics(plhs[0]);
        return;
    }
    
    
    // getDynamicModelsSize(dyn_name)
    if (!strcmp("getDynamicModelsSize", cmd)) {
        // Checking input parameters
        checkInputArgs(cmd,nrhs,3);
        
        char *dyn_name=mxArrayToString(prhs[2]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *out_size=mxGetPr(plhs[0]);
        vle_instance->getDynamicModelsSize(out_size,dyn_name);
        return;
    }
    
    // listDynamicModels(dyn_name)
    if (!strcmp("listDynamicModels", cmd)) {
        // Checking input parameters
        checkInputArgs(cmd,nrhs,3);
        
        char *dyn_name=mxArrayToString(prhs[2]);
        //
        double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        vle_instance->getDynamicModelsSize(out_size,dyn_name);
        
        plhs[0] = mxCreateCellMatrix(1,(int)*out_size);
        vle_instance->listDynamicModels(plhs[0],dyn_name);
        return;
    }
    
    
    // getAtomicModelId(model_name)
    if (!strcmp("getAtomicModelId", cmd)) {
        // Checking input parameters
        checkInputArgs(cmd,nrhs,3);
        char* model_name=mxArrayToString(prhs[2]);
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *model_id=mxGetPr(plhs[0]);
        vle_instance->getAtomicModelId(model_name,model_id);
        return;
	}
    
    // getModelObservables(model_name)
    if (!strcmp("getModelObservables", cmd)) {
        // Checking input parameters
		checkInputArgs(cmd,nrhs,3);
		char* model_name=mxArrayToString(prhs[2]);
		char* obs_name=mxArrayToString(mxCreateString(""));
		vle_instance->getModelObservables(model_name,obs_name);
		plhs[0] = mxCreateString(obs_name);
		return;
	}
    
    // setModelObservables(model_name,obs_name)
    if (!strcmp("setModelObservables", cmd)) {
        // Checking input parameters
		checkInputArgs(cmd,nrhs,4);
		char* model_name=mxArrayToString(prhs[2]);
		char* obs_name=mxArrayToString(prhs[3]);
		vle_instance->setModelObservables(model_name,obs_name);
		return;
	}
	
	
	// getOutputPortNumber(model_name)
	 if (!strcmp("getOutputPortNumber", cmd)) {
	    // Checking input parameters
		checkInputArgs(cmd,nrhs,3);
		char* model_name=mxArrayToString(prhs[2]);
		plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
		double *port_number=mxGetPr(plhs[0]);
			
		if (vle_instance->existGraphModel(model_name)){	
			vle_instance->getOutputPortNumber(model_name,port_number);
		}else{
			*port_number=mxGetNaN();
		}
		return;
	}
	
	// existOutputPort(model_name,port_name)
	if (!strcmp("existOutputPort", cmd)) {
        // Checking input parameters
		checkInputArgs(cmd,nrhs,4);
		char* model_name=mxArrayToString(prhs[2]);
		char* port_name=mxArrayToString(prhs[3]);
		plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
		
		double *port_exists=mxGetPr(plhs[0]);
		if (vle_instance->existGraphModel(model_name)){	
			vle_instance->existOutputPort(model_name,port_name,port_exists);
		}else{
			*port_exists=mxGetNaN();
		}
		return;
	}
	
	// listOutputPorts
	if (!strcmp("listOutputPorts", cmd)) {
		checkInputArgs(cmd,nrhs,3);
        char* model_name=mxArrayToString(prhs[2]);
        
        double *port_number=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
        
        if (vle_instance->existGraphModel(model_name)){	
			vle_instance->getOutputPortNumber(model_name,port_number);	
			plhs[0] = mxCreateCellMatrix(1,(int)*port_number);
			vle_instance->listOutputPorts(plhs[0],model_name);
		}else{
			plhs[0] = mxCreateCellMatrix(1,1);
		}
		
		return;
	
	}
	
	// addOutputPort(model_name,port_name)
	if (!strcmp("addOutputPort", cmd)) {
        // Checking input parameters
		checkInputArgs(cmd,nrhs,4);
		char* model_name=mxArrayToString(prhs[2]);
		char* port_name=mxArrayToString(prhs[3]);
		
		if (vle_instance->existGraphModel(model_name)){		
			vle_instance->addOutputPort(model_name,port_name);
		}
		return;
	}
	
    
    // delOutputPort(model_name,port_name)
    if (!strcmp("delOutputPort", cmd)) {
        // Checking input parameters
		checkInputArgs(cmd,nrhs,4);
		char* model_name=mxArrayToString(prhs[2]);
		char* port_name=mxArrayToString(prhs[3]);
		
		if (vle_instance->existGraphModel(model_name)){	
			vle_instance->delOutputPort(model_name,port_name);
		}	
		return;
	}
	
    
    // run   :  simple run
    if (!strcmp("run", cmd)) {
        // Checking input parameters
        checkInputArgs(cmd,nrhs,2);
        
        switch (nrhs){
            case 2:
                // ajout verif plugin : file !!!!
                vle_instance->run();
                break;
            case 3: // with return_type
				// getting views size
				plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
				double *views_size=mxGetPr(plhs[0]);
				vle_instance->getViewsSize(views_size);
                // Call the method
                plhs[0] =mxCreateCellMatrix(1,(int)*views_size);
                char *return_type=mxArrayToString(prhs[2]);
                // mexPrintf("%s\n run : return_type",return_type);
                // TODO : check return_type .... "CELL_MATRIX", "DOUBLE_MATRIX" (see convert.cpp);
                
                vle_instance->run(plhs[0],return_type);
                break;
                //default:
                // TODO : define action : raise an exception ?
        }
        
        return;
    }
    
    
    
    // existMethod(method_name)
    if (!strcmp("existMethod", cmd)) {
		// Checking input parameters
		checkInputArgs(cmd,nrhs,3);
        
        char *method_name=mxArrayToString(prhs[2]);
        
        plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL);
        double *status=mxGetPr(plhs[0]);
        vle_instance->existMethod(status,method_name);
        return;
    }
    
    // Got here, so command not recognized
    mexErrMsgTxt("Unknown command !");
}


