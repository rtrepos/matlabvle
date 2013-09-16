
#include <cassert>
/*#include <vle/utils/Package.hpp>
#include <vle/utils/Path.hpp>
#include <vle/utils/Module.hpp>*/
#include <vle/utils.hpp>
#include <vle/manager/Run.hpp>
#include <vle/manager/Manager.hpp>
#include <vle/manager/VLE.hpp>
#include <vle/manager/Types.hpp>
#include <vle/vpz/Vpz.hpp>


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
    };

    Vle(char* pkgname, char* filename)
    {
       setFileName(filename);
       setPackageName(pkgname);
       init();
       openFile();
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
    void getExperimentName(char expe_name[100]){
        //       	
       	std::string expname(getVpz()->project().experiment().name());
        strcpy(expe_name, (char*)expname.c_str());
    }

    // getHomeDir
    void getHomeDir(char home_dir[100]){
         //
         strcpy(home_dir, this->mHomeDir);
    }

    // initHomeDir
    void initHomeDir(){
       // Setting vle home dir (VLE_HOME env var)
       strcpy(mHomeDir, getenv("VLE_HOME"));
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

   // setOutputPlugin with location arg.
   void setOutputPluginBase(char* out_view,char* out_location,char* out_destination, char* out_type)
   {
     assert(out_view);
     assert(out_location);
     assert(out_destination);
     assert(out_type);
     
     vpz::Output& out(getVpz()->project().experiment().views().outputs().get(out_view));

     //
     if (strcmp(out_destination, "local") == 0) {
         out.setLocalStream(out_location, out_type);
     } else{
         out.setDistantStream(out_location, out_type);
     }
   }

   // setOutputPlugin without location arg.
   void setOutputPlugin(char* out_view,char* out_destination, char* out_type)
   {
      char* out_location=(char*)"";
      setOutputPluginBase(out_view,out_location,out_destination, out_type);
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
           mexPrintf("\n String list is empty");
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
	size_val[0] = (double)v.size();
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
                                          //mxSetCell(ret,n,mxCreateLogicalScalar(vle::value::toBoolean(*it)));
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


      void getConditionPortValues(mxArray *ret, const char *cond_name,const char *port_name, int idx)
     {
           
         double *size_val;
         getConditionPortValuesSize(size_val,cond_name,port_name);
         mxArray *ret_values=mxCreateCellMatrix((mwSize)(int)*size_val, (mwSize)1);
         getConditionPortValues(ret_values,cond_name,port_name);
     }

     // TODO: getConditionPortValue(cond_name,p_name,idx)
     /*void getConditionPortValue(mxArray* ret, const char *cond_name,const char *port_name,int idx)
     {
           assert(conditionname);
	   assert(portname);
	   
	   std::string conditionname_str(cond_name);
	   std::string portname_str(port_name);
	   
	  .....
     }*/

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

		   int n = 0;
		   for (vle::value::VectorValue::const_iterator it = vals.begin();it != vals.end();++it) {
			   if (*it) {
				   switch ((*it)->getType()) {
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
			  } else {
                                 strcpy(value_type,(char*)"none");
			  }
			  n++;
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

     // TODO: addBooleanCondition(condition_name,port_name,value_content)

     // addStringCondition
     void addStringCondition(double *ret,const char* conditionname, const char* portname, const char* value)
     {
	assert(conditionname);
	assert(portname);
	//Cast of inputs
	//vpz::Vpz*  file(reinterpret_cast < vpz::Vpz* >(handle));
	std::string conditionname_str(conditionname);
	std::string portname_str(portname);
	//Add real
	*ret = -1;
	try {
		//vle::vpz::Condition& cnd(file->project().experiment().conditions().get(conditionname_str));
                vle::vpz::Condition& cnd(getVpz()->project().experiment().conditions().get(conditionname_str));
		cnd.addValueToPort(portname_str, vle::value::String::create(value));
	
	} catch(const std::exception& e) {
		*ret=0;
	}
      }

     // TODO: setConditionValue(condition_name,port_name,num2str(new_value),value_type,int16(value_position))


     // getViewOutput
     void getViewOutput(char *view_name)
   {    
        //
        vpz::View& view(getVpz()->project().experiment().views().get(view_name));
        strcpy(view_name,(char*)view.output().c_str());
   }


     // TODO: existObservable(obs_name)

     // TODO: listObservables

     // TODO: clearObservables()

     // TODO: addObservable(obs_name)

     // TODO: delObservable(obs_name)

     // TODO: listObservablePorts(obs_name)

     // TODO: addObservablePort(obs_name,port_name)

     // TODO: delObservablePort(obs_name,port_name)

     void getDynamicsSize(double *size_val)
   {    
        vpz::DynamicList& lst(getVpz()->project().dynamics().dynamiclist());
        size_val[0]=(double)lst.size();
   }

   // listDynamics
    void listDynamics(mxArray* ret)
   {    
        vpz::DynamicList& lst(getVpz()->project().dynamics().dynamiclist());
	//getting conditions names
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
           mexPrintf("\n String list is empty");
        }
   }

   // run for storage plugin 
   void run(mxArray* ret ,const char *return_type)
   {
	assert(return_type);
        // std::cout << "run: return_type : " << return_type << std::endl;
	
	try {
		// Simulate 1 simulation
		//vle::utils::ModuleManager mm;
		//vle::manager::RunQuiet jrm(mm);
                vle::manager::RunQuiet jrm;
		jrm.start(*getVpz());
		const vle::oov::OutputMatrixViewList& result1(jrm.outputs());
		// Cast of outputs
		//ret = outputMatrixViewListToMxArray(result1,return_type);
                outputMatrixViewListToMxCellArray(ret,result1,return_type);

	} catch(const std::exception& e) {
		mexPrintf("\n matlabvle : ERROR while running simulation, message : \n %s \n",e.what());
                ret=NULL;
	}
    }

   // run for file plugin
   void run()
   {
	std::cout << "dans run()" << std::endl;
	
	//Declaration of outputs
	
	try {
		// Simulate 1 simulation
		//vle::utils::ModuleManager mm;
		//vle::manager::RunQuiet jrm(mm);
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
           mexPrintf("\n String list is empty");
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
           mexPrintf("\n String list is empty");
       }
    }

    

private:
    matlabvle_t mVpz;
    char mFileName[100];
    char mPackageName[100];
    bool mInitStatus;
    char mHomeDir[100];
};




void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{	
    // Get the command string
    char cmd[64];
	if (nrhs < 1 || mxGetString(prhs[0], cmd, sizeof(cmd)))
		mexErrMsgTxt("First input should be a command string less than 64 characters long.");
    
        
    // New
    if (!strcmp("new", cmd)) {
        // Check parameters
        if(nlhs != 1)
            mexErrMsgTxt("New: One output expected.");
        // Return a handle to a new C++ instance
        // 
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
    
    // Check there is a second input, which should be the class instance handle
    if (nrhs < 2)
		mexErrMsgTxt("Second input should be a class instance handle.");
    
    // Delete
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


    // Call the various class methods
    // setFileName    
    if (!strcmp("setFileName", cmd)) {
        // Check parameters
        if (nrhs < 2)
            mexErrMsgTxt("setFilename: missing arguments.");
        // Call the method
         char *in_name=mxArrayToString(prhs[2]);
	
        vle_instance->setFileName(in_name);
        return;
    }

    // getFileName   :  
    if (!strcmp("getFileName", cmd)) {
        // Check parameters
        // Call the method
        char out_name[100];
        vle_instance->getFileName(out_name);
        plhs[0] = mxCreateString(out_name);
        return;
    }

    // setPackageName    
    if (!strcmp("setPackageName", cmd)) {
        // Check parameters
        if (nrhs < 2)
            mexErrMsgTxt("Test: missing arguments.");
        // Call the method
        
        char* in_name=mxArrayToString(prhs[2]);
        vle_instance->setPackageName(in_name);
        return;
    }

    // getPackageName   :  
    if (!strcmp("getPackageName", cmd)) {
        // Check parameters
        // Call the method
        char out_name[100];
        vle_instance->getPackageName(out_name);
        plhs[0] = mxCreateString(out_name);
        return;
    }

    // openFile    
    if (!strcmp("openFile", cmd)) {
        // Call the method
        vle_instance->openFile();
        return;
    }
    
     // getInitStatus   :  
    if (!strcmp("getInitStatus", cmd)) {
        // Check parameters
        if (nlhs < 0 || nrhs < 2)
            mexErrMsgTxt("Test: Unexpected arguments.");
        // Call the method
        bool *out_status;
	vle_instance->getInitStatus(out_status);
        plhs[0]=mxCreateLogicalScalar(*out_status);
        return;
    }

    // getHomeDir   :  
    if (!strcmp("getHomeDir", cmd)) {
        // Check parameters
        
        // Call the method
        char home_dir[100];
        vle_instance->getHomeDir(home_dir);

        plhs[0] = mxCreateString(home_dir);
        return;
    }


     // getExperimentName
     if (!strcmp("getExperimentName", cmd)) {
        // Check parameters
        
        // Call the method
        char expe_name[100];
        vle_instance->getExperimentName(expe_name);

        plhs[0] = mxCreateString(expe_name);
        return;
    }


    // getBegin
     if (!strcmp("getBegin", cmd)) {
         // Check parameters
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *out_begin=mxGetPr(plhs[0]);
         vle_instance->getBegin(out_begin);
         return;
     }


     // setBegin
     if (!strcmp("setBegin", cmd)) {
         // Check parameters
         if (nrhs < 3)
            mexErrMsgTxt("Test: missing arguments.");

         double in_begin=mxGetScalar(prhs[2]);
         vle_instance->setBegin(in_begin);
         return;
     }


     // getDuration
     if (!strcmp("getDuration", cmd)) {
         // Check parameters
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *out_duration=mxGetPr(plhs[0]);
         vle_instance->getDuration(out_duration);
         return;
     }


     // setDuration
     if (!strcmp("setDuration", cmd)) {
         // Check parameters
         if (nrhs < 3)
            mexErrMsgTxt("Test: missing arguments.");

         double in_duration=mxGetScalar(prhs[2]);
         vle_instance->setDuration(in_duration);
         return;
     }


    // setOutputPlugin   :  
    if (!strcmp("setOutputPlugin", cmd)) {
        // Check parameters
        if (nrhs < 5)
            mexErrMsgTxt("Test: missing arguments.");
        
        char* out_view=mxArrayToString(prhs[2]);
        char* out_destination=mxArrayToString(prhs[3]);
        char* out_type=mxArrayToString(prhs[4]);
        vle_instance->setOutputPlugin(out_view,out_destination,out_type);
        return;
    }

     // getConditionsSize
     if (!strcmp("getConditionsSize", cmd)) {
         // Check parameters
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *out_size=mxGetPr(plhs[0]);
         vle_instance->getConditionsSize(out_size);
         return;
     }


     // listConditions
     if (!strcmp("listConditions", cmd)) {
         // get conditions list size
         double *max_cond=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
         vle_instance->getConditionsSize(max_cond);
         
         plhs[0] = mxCreateCellMatrix(1,(int)*max_cond);
         vle_instance->listConditions(plhs[0]);
	
         return;
     }


     // getConditionPortsSize
     if (!strcmp("getConditionPortsSize", cmd)) {
         if (nrhs < 3)
            mexErrMsgTxt("Test: missing arguments.");
         
         char *cond_name=mxArrayToString(prhs[2]);
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *out_size=mxGetPr(plhs[0]);
         vle_instance->getConditionsPortsSize(out_size,cond_name);
         return;
     }

     // listConditionPorts(cond_name)
     if (!strcmp("listConditionPorts", cmd)) {
         if (nrhs < 3)
            mexErrMsgTxt("Test: missing arguments.");
         
         char *cond_name=mxArrayToString(prhs[2]);
         double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
         vle_instance->getConditionsPortsSize(out_size,cond_name);

         plhs[0] = mxCreateCellMatrix(1,(int)*out_size);
         vle_instance->listConditionPorts(plhs[0],cond_name);
         return;
     }

     // clearConditionPort(condition_name,port_name)
 
     if (!strcmp("clearConditionPort", cmd)) {
         if (nrhs < 4)
            mexErrMsgTxt("Test: missing arguments.");

         char *cond_name=mxArrayToString(prhs[2]);
         char *port_name=mxArrayToString(prhs[3]);
         
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *status=mxGetPr(plhs[0]);
         vle_instance->clearConditionPort(status,cond_name,port_name);
         return;
     }

     // getConditionPortValuesSize(cond_name,p_name)
     if (!strcmp("getConditionPortValuesSize", cmd)) {
         if (nrhs < 4)
            mexErrMsgTxt("Test: missing arguments.");

         char *cond_name=mxArrayToString(prhs[2]);
         char *port_name=mxArrayToString(prhs[3]);

         
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *out_size=mxGetPr(plhs[0]);
         vle_instance->getConditionPortValuesSize(out_size,cond_name,port_name);
         return;
     }

     // getConditionPortValues(cond_name,p_name)
     if (!strcmp("getConditionPortValues", cmd)) {
         if (nrhs < 4)
            mexErrMsgTxt("Test: missing arguments.");

         char *cond_name=mxArrayToString(prhs[2]);
         char *port_name=mxArrayToString(prhs[3]);
         double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));

         vle_instance->getConditionPortValuesSize(out_size,cond_name,port_name);
         
         plhs[0]=mxCreateCellMatrix((mwSize)(int)*out_size, (mwSize)1); 

         vle_instance->getConditionPortValues(plhs[0],cond_name,port_name);
         return;
     }

     // TODO: COMPLETE getConditionPortValue(cond_name,p_name,idx)
     if (!strcmp("getConditionPortValue", cmd)) {
         if (nrhs < 5)
            mexErrMsgTxt("Test: missing arguments.");
         char value_type[30];
         char *cond_name=mxArrayToString(prhs[2]);
         char *port_name=mxArrayToString(prhs[3]);
         double value_pos=mxGetScalar(prhs[4]);
         
         vle_instance->getConditionValueType(value_type,cond_name,port_name,(int) value_pos);

         std::cout << value_type << std::endl;

         mxArray *val;
         vle_instance->getConditionPortValues(val,cond_name,port_name,(int) value_pos);
         // TODO: return value of the right type !!!!!
         //plhs[0]=val;

         return;
     }


     // getConditionValueType(cond_name,port_name,position_value)
     if (!strcmp("getConditionValueType", cmd)) {
         if (nrhs < 4)
            mexErrMsgTxt("Test: missing arguments.");

         char value_type[30];
         char *cond_name=mxArrayToString(prhs[2]);
         char *port_name=mxArrayToString(prhs[3]);
         double value_pos=mxGetScalar(prhs[4]);
         
         vle_instance->getConditionValueType(value_type,cond_name,port_name,(int)value_pos);
         
         plhs[0] = mxCreateString(value_type);
         return;
     }

     // addRealCondition(condition_name,port_name,value_content)
     if (!strcmp("addRealCondition", cmd)) {
         if (nrhs < 5)
            mexErrMsgTxt("Test: missing arguments.");

         char *cond_name=mxArrayToString(prhs[2]);
         char *port_name=mxArrayToString(prhs[3]);
         double value=mxGetScalar(prhs[4]);
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *status=mxGetPr(plhs[0]);
         vle_instance->addRealCondition(status,cond_name,port_name,value);
         return;
     }
     // addIntegerCondition(condition_name,port_name,value_content)
     if (!strcmp("addIntegerCondition", cmd)) {
         if (nrhs < 5)
            mexErrMsgTxt("Test: missing arguments.");

         char *cond_name=mxArrayToString(prhs[2]);
         char *port_name=mxArrayToString(prhs[3]);
         double value=mxGetScalar(prhs[4]);
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *status=mxGetPr(plhs[0]);
         vle_instance->addRealCondition(status,cond_name,port_name,(int)value);
         return;
     }

     // TODO: addBooleanCondition(condition_name,port_name,value_content)

     // addStringCondition(condition_name,port_name,value_content)
     if (!strcmp("addStringCondition", cmd)) {
         if (nrhs < 5)
            mexErrMsgTxt("Test: missing arguments.");

         char *cond_name=mxArrayToString(prhs[2]);
         char *port_name=mxArrayToString(prhs[3]);
         char *value=mxArrayToString(prhs[4]);
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *status=mxGetPr(plhs[0]);
         vle_instance->addStringCondition(status,cond_name,port_name,value);
         return;
     }

     // TODO: setConditionValue(condition_name,port_name,num2str(new_value),value_type,int16(value_position))


     // getViewOutput
     if (!strcmp("getViewOutput", cmd)) {
        //
        if (nlhs < 0 || nrhs < 3)
            mexErrMsgTxt("Test: missing arguments.");
        //
        char *view_name=mxArrayToString(prhs[2]);
        vle_instance->getViewOutput(view_name);
        plhs[0] = mxCreateString(view_name);
        return;
    }

     // TODO: existObservable(obs_name)

     // TODO: listObservables

     // TODO: clearObservables()

     // TODO: addObservable(obs_name)

     // TODO: delObservable(obs_name)

     // TODO: listObservablePorts(obs_name)

     // TODO: addObservablePort(obs_name,port_name)

     // TODO: delObservablePort(obs_name,port_name)


     // getDynamicsSize
     if (!strcmp("getDynamicsSize", cmd)) {
         // Check parameters
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *out_size=mxGetPr(plhs[0]);
         vle_instance->getDynamicsSize(out_size);
         return;
     }


     // listDynamics
     if (!strcmp("listDynamics", cmd)) {
         // get conditions list size
         
         double *max_cond=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
         vle_instance->getDynamicsSize(max_cond);
         
         plhs[0] = mxCreateCellMatrix(1,(int)*max_cond);
         vle_instance->listDynamics(plhs[0] );
	 //plhs[0] = ret;
         return;
     }

     //
     // getDynamicModelsSize
     if (!strcmp("getDynamicModelsSize", cmd)) {
         if (nrhs < 3)
            mexErrMsgTxt("Test: missing arguments.");
         
         char *dyn_name=mxArrayToString(prhs[2]);
         plhs[0]=mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL); 
         double *out_size=mxGetPr(plhs[0]);
         vle_instance->getDynamicModelsSize(out_size,dyn_name);
         return;
     }

     // listDynamicModels(dynamic_name)
     if (!strcmp("listDynamicModels", cmd)) {
         if (nrhs < 3)
            mexErrMsgTxt("Test: missing arguments.");

         char *dyn_name=mxArrayToString(prhs[2]);
         //
         double *out_size=mxGetPr(mxCreateDoubleMatrix((mwSize)1, (mwSize)1, mxREAL));
         vle_instance->getDynamicModelsSize(out_size,dyn_name);

         plhs[0] = mxCreateCellMatrix(1,(int)*out_size);
         vle_instance->listDynamicModels(plhs[0],dyn_name);
         return;
     }


      // NEW METHODS **************************************
      // TODO: addObserbablePortsToView(obj,obs_name,view_name,vc_port_names)

      // TODO: removeObservablePortsFromView(obj,obs_name,view_name,vc_port_names)
     


     // run   :  simple run 
    if (!strcmp("run", cmd)) {
        // Check parameters
        switch (nrhs){
            case 2:
                // ajout verif plugin : file !!!!
                vle_instance->run();
                break;
            case 3: // with return_type
                // Call the method
                // No dimensions are specified , single run !!!!
                mxArray* outData=mxCreateCellMatrix(1,1);
                char *return_type=mxArrayToString(prhs[2]);
                // TODO : check return_type .... "CELL_MATRIX", "DOUBLE_MATRIX" (see convert.cpp);
                vle_instance->run(outData,return_type);
                plhs[0] = outData;
                break;
            //default:
               // TODO : define action : raise an exception ?
        }
        
        return;
    }
    

    // Got here, so command not recognized
    mexErrMsgTxt("Command not recognized.");
}


