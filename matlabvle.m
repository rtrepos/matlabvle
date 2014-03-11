
classdef (ConstructOnLoad) matlabvle_interface < dynamicprops
    properties (SetAccess = private, Hidden = true)
        % Handle to the underlying C++ class instance
        objectHandle;
        % Template of anonymous function handle
        method_code='@(varargin) matlabvle_interface.call(uint64(object_handle),''name'',varargin{:})';
        % C++ methods names to be added dynamically as properties
        % containing a handle to a dedicated anonymous function
        methods_names={'getFileName' 'setFileName' 'getPackageName' 'setPackageName'...
            'openFile' 'getStatus' 'getHomeDir' ...
            'getExperimentName' 'getBegin' 'setBegin' 'getDuration' 'setDuration' ...
            'setOutputPlugin' 'getConditionsSize' 'listConditions' 'getConditionPortsSize'...
            'listConditionPorts' 'clearConditionPort' 'getConditionPortValuesSize'...
            'getConditionPortValues' 'getConditionPortValue' 'getConditionValueType' ...
            'addRealCondition' 'addIntegerCondition' 'addStringCondition' ...
            'getViewOutput' 'getDynamicsSize' 'listDynamics'...
            'getDynamicModelsSize' 'listDynamicModels'...
            ...
            'run'};
    end
    
    methods (Static)
        % Calling C++ matlabvle_interface_mex methods
        function varargout=call(handle_id,name,varargin)
            [varargout{1:nargout}] = matlabvle_interface_mex(name, handle_id, varargin{:});
        end
    end
    
    methods     
        function this = matlabvle_interface(varargin)
            % Constructor - Create a new C++ Vle class instance
            % Handle to C++ instance
            this.objectHandle = matlabvle_interface_mex('new', varargin{:});
            % Adding dynamic properties, each containing a handle to a dedicated
            % anonymous function calling a c++ method
            for i=1:length(this.methods_names)
                method_name=this.methods_names{i};
                if this.existMethodName(method_name)
                    this.addDynProp(method_name,this.getMethodHandle(method_name));
                else
                    fprintf('%s : unreferenced method name !',method_name); 
                    disp('Use obj.getMethodsNames to get complete list of interfaced methods!');
                end
            end
        end
            
        function delete(this)
            % Destructor - Destroy the C++ Vle class instance
            matlabvle_interface_mex('delete', this.objectHandle);
        end
         
        %%
        function addDynProp(this,prop_name,prop_value)
            % Adding a dynamic property and set its value
            % to a matlabvle_interface instance.
            % prop_name: property name
            % prop_value : property value
            if ~isprop(this,prop_name)
                new_prop = addprop(this,prop_name);
                new_prop.Transient=true;
                % access restriicted to class members
                new_prop.SetAccess='private';
                this.(prop_name)=prop_value;
            else
                fprintf('\n %s : this is an already defined property !',prop_name);
            end
        end
        
        function method_handle = getMethodHandle(this,method_name)
            % Formatting method handle from method_code property
            % and method name
            % method_name : name of a declared method of underlying C++
            % class in methods_names property
            % method_handle : handle to a specific anonymous function
            if this.existMethodName(method_name)
                % adding checking if method name exists
                new_method_str=strrep(this.method_code,'object_handle',num2str(this.objectHandle));
                method_handle=str2func(strrep( new_method_str,'name',method_name));
            else
                method_handle=[];
            end
        end
        
        function methods_list = getMethodsNames(this)
            % Getting declared method names list
            % from methods_name property
            methods_list = this.methods_name;
        end
        
        function method_exists = existMethodName(this,method_name)
            % Evaluates if a method name is declared in class
            % method_name : name of a method
            % method_exists : true if exists, false instead
           method_exists=ismember(method_name,this.methods_names);
        end
        
    end
end
