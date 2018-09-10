import os,json
from optparse import OptionParser
from amlrealtimeai import DeploymentClient

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
    from amlrealtimeai import resnet50
    import amlrealtimeai.resnet50.utils

    # Input images as a two-dimensional tensor containing an arbitrary number of images represented a strings
    in_images = tf.placeholder(tf.string)
    image_tensors = resnet50.utils.preprocess_array(in_images)
    if verbose: print(image_tensors.shape)

    # Featurizer
    from amlrealtimeai.resnet50.model import LocalQuantizedResNet50
    model_path = os.path.expandvars('$CMSSW_BASE/src/Jet2011/AnalysisFW/python')
    model = LocalQuantizedResNet50(model_path)
    if verbose: print(model.version)

    # Classifier
    model.import_graph_def(include_featurizer=False)
    if verbose: print(model.classifier_input.shape)

    # service definition
    from amlrealtimeai.pipeline import ServiceDefinition, TensorflowStage, BrainWaveStage

    save_path = model_path
    service_def_path = os.path.join(save_path, 'service_def_'+model_name+'.zip')

    service_def = ServiceDefinition()
    service_def.pipeline.append(TensorflowStage(tf.Session(), in_images, image_tensors))
    service_def.pipeline.append(BrainWaveStage(model))
    service_def.pipeline.append(TensorflowStage(tf.Session(), model.classifier_input, model.classifier_output))
    service_def.save(service_def_path)
    if verbose: print(service_def_path)
    
    return service_def_path

parser = OptionParser()
parser.add_option("-p", "--params", dest="params", default="service_model_params.json", help="name of service & model params output json file (default = %default)")
parser.add_option("-m", "--model", dest="model", default="", help="use model with provided name (default = %default)")
parser.add_option("-r", "--recreate", dest="recreate", default=False, action="store_true", help="recreate model (instead of use existing model)")
parser.add_option("-s", "--service", dest="service", default="", help="use service with provided name (default = %default)")
parser.add_option("-d", "--delete", dest="delete", default=False, action="store_true", help="delete service and/or model (instead of starting)")
parser.add_option("-v", "--verbose", dest="verbose", default=False, action="store_true", help="turn on informational printouts")
(options, args) = parser.parse_args()

# create json file if needed
open(options.params,'a').close()

subscription_id = "80defacd-509e-410c-9812-6e52ed6a0016"
resource_group = "CMS_FPGA_Resources"
model_management_account = "CMS_FPGA_1"

model_name = options.model
service_name = options.service

deployment_client = DeploymentClient(subscription_id, resource_group, model_management_account)

if len(model_name)>0:
    if options.delete:
        # use stored model id
        model_id = get_from_json(options.params,"model_id")
        deployment_client.delete_model(model_id)
    elif options.recreate:
        service_def_path = define_model(model_name,options.verbose)
        model_id = deployment_client.register_model(model_name, service_def_path)

        # store model id in json file
        update_json(options.params,{"model_id": model_id})
    else:
        # from list of models, pick newest one with the provided name
        models = [x for x in deployment_client.list_models() if x.name==model_name]
        import dateutil.parser
        model_id = sorted(models,key=lambda x: dateutil.parser.parse(x.createdAt))[-1].id

        # store model id in json file
        update_json(options.params,{"model_id": model_id})
else:
    # use stored model id
    model_id = get_from_json(options.params,"model_id")

if len(service_name)>0:
    if options.delete:
        services = deployment_client.list_services()
        for service in filter(lambda x: x.name == service_name, services):
            print("Deleting: "+str(service.id))
            deployment_client.delete_service(service.id)
    else:
        service = deployment_client.get_service_by_name(service_name)

        if(service is None):
            service = deployment_client.create_service(service_name, model_id)    
        else:
            service = deployment_client.update_service(service.id, model_id)

        update_json(options.params, {"address": service.ipAddress, "port": service.port})
