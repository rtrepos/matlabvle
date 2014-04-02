
classdef (ConstructOnLoad) matlabvle < dynamicprops
    %
    % matlabvle Properties:
    %  objectHandle - Handle to the underlying C++ class instance
    %  method_code - Template for creating anonymous function handle
    %  methods_names - Cell of method names to be added as dynamic properties
    %
    % matlabvle Methods:
    %  callMethod - static, for executing any C++ class method through added dynamic
    %  properties
    %  listMethods - display methods list (added dynamically from
    %  methods_names
    %  matlabvle - Class constructor
    %  delete - Class destructor
    %  getHandle - getting the object handle
    %  addDynMethods - Adding dynamic properties with an anonymous function
    %  handle as value
    %  addDynProp - adding a single dynamic property
    %  getMethodHandle - formatting an anonymous function for a specific
    %  method name
    %  getMethodsNames - Getting active methods list
    %  existMethodName - Evaluating if a method exists in C++ Vle class
    
    properties (SetAccess = private, Hidden = true)
        % handle to class instance
        objectHandle;
        % anonymous function template
        method_code='@(varargin) matlabvle.callMethod(uint64(object_handle),''name'',varargin{:})';
    end
    
    properties (Constant=true, Hidden=true)
        % C++ methods names to be added dynamically as properties
        % each containing a handle to a dedicated anonymous function
        % executing a method of the instanciated C++ Vle class of 
        % matlab_vle_mex.cpp
        %
        methods_names={'getFileName' 'setFileName' 'getPackageName' 'setPackageName'...
            'openFile' 'save' 'getStatus' 'getHomeDir' ...
            'getExperimentName' 'getBegin' 'setBegin' 'getDuration' 'setDuration' ...
            'setOutputPlugin' 'getOutputPlugin' ...
            'addCondition' 'delCondition' 'getConditionsSize' 'listConditions' 'getConditionPortsSize'...
            'listConditionPorts' 'clearConditionPort' 'getConditionPortValuesSize'...
            'getConditionPortValues' 'getConditionPortValue' 'getConditionValueType' ...
            'addRealCondition' 'addIntegerCondition' 'addStringCondition' 'addBooleanCondition'...
            'getViewOutput' 'getViewsSize' 'listViews' 'existView' 'addView' ...
            'getDynamicsSize' 'listDynamics' 'getDynamicModelsSize' 'listDynamicModels' 'setConditionValue' ...
            'existObservable' 'listObservables' 'getObservablesSize' 'clearObservables' ...
            'addObservable' 'delObservable' ...
            'listObservablePorts' 'getObservablePortsSize' 'addObservablePort' 'delObservablePort' ...
            'existObservablePortView' 'addViewToObservablePort' 'delViewFromObservablePort' ...
            'getAtomicModelId' 'getModelObservables' 'setModelObservables'... 
            'getOutputPortNumber' 'existOutputPort' 'addOutputPort' 'delOutputPort' 'listOutputPorts'...
            'run' ...
            'existMethod'};
    end
    
    methods (Static)
        function varargout=callMethod(handle_id,name,varargin)
            %
            % callMethod(handle_id,name,varargin)
            %
            %   Calling C++ matlabvle_interface_mex methods
            %      handle_id - object handle
            %      name - method name
            %      varargin : method input arguments
            
            [varargout{1:nargout}] = matlabvle_interface_mex(name, handle_id, varargin{:});
        end
        
        
        function listMethods()
            %
            % listMethods()
            % 
            %   Diplaying dynamically added methods list
            %
            list=matlabvle.methods_names;
            sprintf('%s\n',list{:})
        end
    end
    
    methods     
        function this = matlabvle(varargin)
            %
            % matlabvle()
            %
            %   Constructor - Create a new C++ Vle class instance
            
            % Handle to C++ instance
            this.objectHandle = matlabvle_interface_mex('new', varargin{:});
            % Adding C++ Vle class methods as dynamic properties
            this=addDynMethods(this);
        end
        
            
        function delete(this)
            %
            % delete()
            %
            %    Destructor - Destroy the C++ Vle class instance
            matlabvle_interface_mex('delete', this.getHandle());
        end
        
        
        function obj_handle=getHandle(this)
            %
            % obj_handle=getHandle()
            %
            %   Getting handle to object
            obj_handle=this.objectHandle;
        end
         
        
        function this=addDynMethods(this)
            %
            % addDynMethods()
            %
            %   Adding dynamic properties, each containing a handle to a dedicated
            %   anonymous function calling a c++ method
            %   from methods_names property content
            
            exist_methods=this.existMethodName(this.methods_names);
            for i=1:length(this.methods_names)
                method_name=this.methods_names{i};
                if exist_methods(i)
                    this.addDynProp(method_name,this.getMethodHandle(method_name));
                else
                    fprintf('\n\n%s : unreferenced method name !\n',method_name); 
                    disp('Check C++ file matlabvle_interface_mex.cpp (in setMethodsList method) !');
                end
            end
        end
        
        
        function addDynProp(this,prop_name,prop_value)
            %
            % addDynProp(property_name,property_value)
            %
            %   Adding a dynamic property and set its value
            %   to a matlabvle instance.
            %      prop_name - property name
            %      prop_value - property value
            if ~isprop(this,prop_name)
                new_prop = addprop(this,prop_name);
                new_prop.Transient=true;
                % access restricted to class members
                new_prop.SetAccess='private';
                this.(prop_name)=prop_value;
            else
                fprintf('\n %s : this is an already defined dynamic property !',prop_name);
            end
        end
        
        
        function method_handle = getMethodHandle(this,method_name)
            %
            % getMethodHandle(method_name)
            %
            %   Formatting method handle from method_code property
            %   and method name
            %
            % method_name - name of a declared method of underlying C++
            % class in methods_names property
            % method_handle : handle to a specific anonymous function
            
            if this.existMethodName(method_name)
                % adding checking if method name exists
                new_method_str=strrep(this.method_code,'object_handle',num2str(this.getHandle()));
                method_handle=str2func(strrep( new_method_str,'name',method_name));
            else
                method_handle=[];
            end
        end
        
        
        function methods_list = getMethodsNames(this)
            % 
            % getMethodsNames()
            %
            %   Getting declared method names list
            %   from methods_name property
            
            exist_methods=this.existMethodName(this.methods_names);
            methods_list = this.methods_names(exist_methods);
        end
        
        
        function method_exists = existMethodName(this,method_name)
            %
            % method_exists = existMethodName(method_name)
            %
            %   Evaluates if a method name is declared in class
            %
            %      method_name - name of a method
            %      method_exists - true if exists, false instead
            
            if ischar(method_name)
                method_name={method_name};
            end
            method_exists=logical(cellfun(@(x) matlabvle_interface_mex('existMethod', this.getHandle(),x), method_name));
        end
        
        
%         function set_methodHelp(this,val)
%             if  ~(ischar(val))
%                 error('methodHelp requires a char input ')
%             end
%             this.methodHelp = val; % set property value
%         end


    end
end
