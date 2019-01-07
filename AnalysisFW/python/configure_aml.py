import os,json
from optparse import OptionParser
from azureml.core import Workspace
from azureml.core.model import Model
from azureml.core.webservice import Webservice
from azureml.exceptions import WebserviceException
from azureml.contrib.brainwave import BrainwaveWebservice, BrainwaveImage

# setting for keras
os.environ['KERAS_BACKEND'] = 'tensorflow'

def update_json(fname,updates):
    with open(fname,'r') as pfile:
        # handle empty files
        try:
            pdict = json.load(pfile)
        except ValueError:
            pdict = {}
    for key in updates:
        pdict[key] = updates[key]
    with open(fname,'w') as pfile:
        json.dump(pdict,pfile)    

def get_from_json(fname,key):
    with open(fname,'r') as pfile:
        pdict = json.load(pfile)
    return pdict[key]

def define_model(model_name,verbose=False):
    import tensorflow as tf
    from azureml.contrib.brainwave.models import QuantizedResnet50
    from azureml.contrib.brainwave.pipeline import ModelDefinition, TensorflowStage, BrainWaveStage

    # Featurizer
    model_path = os.path.expandvars('$CMSSW_BASE/src/SonicCMS/AnalysisFW/python')
    model = QuantizedResnet50(model_path, is_frozen = True)
    if verbose: print(model.version)

    # Classifier
    feature_tensor = model.import_graph_def()
    classifier_output = model.get_default_classifier(feature_tensor)
    if verbose: print(model.classifier_input.shape)

    # service definition
    from azureml.contrib.brainwave.pipeline import ModelDefinition, TensorflowStage, BrainWaveStage

    save_path = model_path
    model_def_path = os.path.join(save_path, 'model_def_'+model_name+'.zip')

    model_def = ModelDefinition()
    with tf.Session() as sess:
        model_def.pipeline.append(BrainWaveStage(sess, model))
        model_def.pipeline.append(TensorflowStage(sess, feature_tensor, classifier_output))
        model_def.save(model_def_path)
    if verbose: print(model_def_path)
    
    return model_def_path

parser = OptionParser()
parser.add_option("-p", "--params", dest="params", default="service_model_params.json", help="name of service & model params output json file (default = %default)")
parser.add_option("-m", "--model", dest="model", default="", help="use model with provided name (default = %default)")
parser.add_option("-r", "--recreate", dest="recreate", default=False, action="store_true", help="recreate model (instead of use existing model)")
parser.add_option("-s", "--service", dest="service", default="", help="use service with provided name (default = %default)")
parser.add_option("-d", "--delete", dest="delete", default=False, action="store_true", help="delete service and/or model (instead of starting)")
parser.add_option("-v", "--verbose", dest="verbose", default=False, action="store_true", help="turn on informational printouts")
(options, args) = parser.parse_args()

# create service json file if needed
open(options.params,'a').close()

# info for workspace
subscription_id = "80defacd-509e-410c-9812-6e52ed6a0016"
resource_group = "CMS_FPGA_Resources"
workspace_name = "Fermilab"

# create workspace, save json for later
ws = Workspace(subscription_id = subscription_id, resource_group = resource_group, workspace_name = workspace_name)

model_name = options.model
service_name = options.service

if len(model_name)>0:
    if options.delete:
        # use stored model id
        model_id = get_from_json(options.params,"model_id")
        model = Model(ws,id=model_id)
        model.delete()
    elif options.recreate:
        model_def_path = define_model(model_name,options.verbose)
        model = Model.register(ws, model_def_path, model_name)
        model_id = model.id

        # store model id in json file
        update_json(options.params,{"model_id": model_id})
    else:
        # from list of models, pick newest one with the provided name
        models = [x for x in ws.models() if x.name==model_name]
        import dateutil.parser
        model_id = sorted(models,key=lambda x: dateutil.parser.parse(x.created_time))[-1].id

        # store model id in json file
        update_json(options.params,{"model_id": model_id})
else:
    # use stored model id
    model_id = get_from_json(options.params,"model_id")

service = None
if len(service_name)>0:
    try:
        service = Webservice(ws,service_name)
        if options.delete:
            print("Deleting: "+str(service.id))
            service.delete()
            service = None
    except WebserviceException:
        if not options.delete:
            service = Webservice.deploy_from_model(ws, service_name, [Model(ws,id=model_id)], BrainwaveImage.image_configuration(), BrainwaveWebservice.deploy_configuration())
            service.wait_for_deployment(True)

    if service is not None:
        update_json(options.params, {"address": service.ip_address, "port": service.port})

