import os

import cog

from codegen import parse_and_generate_code as parse
from codegen import category_for_file

import codegen_types.ctypes as ctypes

# Example invocations from tools directiory:
# python -m cogapp -ed -D cog_category_file=publication.h -D model_dir="../model" -o tmp/output.xsd cog/templates/schema.xsd
# python -m cogapp -ed -D cog_category_file=publication.h -D model_dir='../model' -o tmp/output.h cog/templates/category.h 
# python -m cogapp -ed -D cog_category_file=publication.h -D model_dir='../model' -o tmp/publication_node_backend.h cog/templates/node_backend.h
# python -m cogapp -ed -D cog_category_file=publication.h -D model_dir='../model' -o tmp/publication_node_backend.c cog/templates/node_backend.c 


def init_models(model_dir, filename):
    global models
    global category_file
    category_file = filename
    models = parse([model_dir], None, None, None)

def _type_conversion(type_name):
    if type_name == "int":
        return "integer"
    return type_name

def _category_name():
    return category_for_file(category_file, models).struct_name 

def category_name():
    cog.inline()
    cog.out(_category_name())   

def category_xsd():
    category = category_for_file(category_file, models)   
    cog.outl('<xsd:complexType name="{0}">'.format(category.struct_name))
    for name, type_name in category.backend_type_list():
        cog.outl(r'    <xsd:attribute name="{0}" type="xsd:{1}"/>'.format(name, _type_conversion(type_name)))


def node_h_guard():
    category = category_for_file(category_file, models)
    
    _generic_h_guard('{0}_node_backend'.format(category.struct_name))
    
def category_h_guard():
    category = category_for_file(category_file, models)
    
    _generic_h_guard(category.struct_name)
    
def _name_root(filename):
    filename_root, _ = os.path.splitext(filename)
    return filename_root.lower()

def filename_root():
    cog.inline()
    cog.out(_name_root(category_file))

def _generic_h_guard(filename):
    guard = '{0}_h'.format(_name_root(filename))
    
    cog.outl('#ifndef {0}'.format(guard))    
    cog.outl('#define {0}'.format(guard))
    
def category_h_struct():
    category = category_for_file(category_file, models)
    
    cog.outl('struct {0}'.format(category.struct_name)) 
        
def category_h_members():
    category = category_for_file(category_file, models)
    
    for name, type_name in category.backend_type_list():
        cog.outl('{0} {1};'.format(ctypes.from_platform_type(type_name), name))

def node_interface_declaration():
    name = _category_name()
    cog.outl("struct {0}_backend_interface *{1}_node_interface_func();".format(name, name))
    
def h_include():
    cog.outl("#include \"{0}\"".format(category_file.lower()))
    
def _check_pass_filter(name, type_name):    
    if (type_name == "string"):
        _check_string_passes_filter(name)
    elif (type_name == "int"):
        _check_int_passes_filter(name)
    else:
        raise(ValueError('Unexpected type {0}'.format(type_name)))
    
def _check_string_passes_filter(name):    
    cog.outl("    if ((fptr->{0})".format(name))
    cog.outl("    && (strlen( fptr->{0} ) != 0)) {{".format(name))
    cog.outl("        if(!( pptr->{0} ))".format(name))
    cog.outl("            return(0);")
    cog.outl("        else if ( strcmp(pptr->{0},fptr->{1}) != 0)".format(name, name))
    cog.outl("            return(0);")
    cog.outl("        }")
    
def _check_int_passes_filter(name):
    cog.outl("    if (( fptr->{0} ) && ( pptr->{1} != fptr->{2} )) return(0);".format(name, name, name))
    
def pass_category_filter():
    category = category_for_file(category_file, models)
    category_name = _category_name()
    cog.outl("private int pass_{0}_filter(".format(category_name))
    cog.outl("    struct {0} * pptr,struct {1} * fptr) {{".format(category_name, category_name))
    for name, type_name in category.backend_type_list():
        _check_pass_filter(name, type_name)
        
def load_attributes():
    _format_category(category_file,
                     "if ((aptr = document_atribut( vptr, \"{0}\" )) != (struct xml_atribut *) 0)",
                     "    {1}->{0} = document_atribut_string(aptr);",
                     "    {1}->{0} = document_atribut_value(aptr);")
                 
def save_attributes():
    _format_category(category_file,
                     "fprintf(h,\" {0}=%c\",0x0022);",
                     "fprintf(h,\"%s\",(pptr->{0}?pptr->{0}:\"\"));",
                     "fprintf(h,\"%u\",pptr->{0});",
                     "fprintf(h,\"%c\",0x0022);")
        
def clone():
    _format_category(category_file, 
                     "if (original->{0}) {{",
                     "    success = success && (copy->{0} = allocate_string(original->{0}));",
                     "    copy->{0} = original->{0};",
                     "}")                    

def _format_category(category_file, prefix, string_format, int_format, suffix = None):
    category = category_for_file(category_file, models)
    for name, type_name in category.backend_type_list():
        cog.outl(prefix.format(name))
        if (type_name == "string"):
            cog.outl(string_format.format(name, _name_root(category_file)))
        elif(type_name == "int"):
            cog.outl(int_format.format(name, _name_root(category_file)))
        else:
            raise(ValueError('Unexpected type {0}'.format(type_name)))
        if suffix:
            cog.outl(suffix)
    
        