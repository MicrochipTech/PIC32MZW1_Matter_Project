"""
Device Info Retrieval Example
"""
# (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
#
# Subject to your compliance with these terms, you may use Microchip software
# and any derivatives exclusively with Microchip products. It is your
# responsibility to comply with third party license terms applicable to your
# use of third party software (including open source software) that may
# accompany Microchip software.
#
# THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
# EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
# WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
# PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
# SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
# OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
# MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
# FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
# LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
# THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
# THIS SOFTWARE.

from cryptoauthlib import *
from common import *
from ctypes import sizeof, create_string_buffer, memmove, addressof, c_uint8, POINTER
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography import x509
from datetime import datetime
import pytz
import sys

ATCACERT_DEF_DEVICE_CONFIG = {
    'type': atcacert_cert_type_t.CERTTYPE_X509,
    'template_id': 2,
    'chain_id': 0,
    'private_key_slot': 0,
    'ca_cert_def': None,
    'sn_source': atcacert_cert_sn_src_t.SNSRC_STORED,
    'cert_sn_dev_loc': {
        'zone': atcacert_device_zone_t.DEVZONE_DATA,
        'slot': 8,
        'is_genkey': 0,
        'offset': 0,
        'count': 8
    },
    'issue_date_format': atcacert_date_format_t.DATEFMT_RFC5280_UTC,
    'expire_date_format': atcacert_date_format_t.DATEFMT_RFC5280_UTC,
    'tbs_cert_loc': {'offset': 4, 'count': 394},
    'expire_years': 20,
    'public_key_dev_loc': {
        'zone': atcacert_device_zone_t.DEVZONE_DATA,
        'slot': 0,
        'is_genkey': 1,
        'offset': 0,
        'count': 64
    },
    'comp_cert_dev_loc': {
        'zone': atcacert_device_zone_t.DEVZONE_DATA,
        'slot': 10,
        'is_genkey': 0,
        'offset': 0,
        'count': 72
    },
    'std_cert_elements' : [
        {'offset': 239, 'count': 64},
        {'offset': 413, 'count': 74},
        {'offset': 102, 'count': 13},
        {'offset': 117, 'count': 13},
        {'offset': 94, 'count': 4},
        {'offset': 15, 'count': 8},
        {'offset': 381, 'count': 20},
        {'offset': 348, 'count': 20},
    ]
}

ATCACERT_DEF_DEVICE_TEMPLATE_VECTOR = bytearray([
  0x30, 0x82, 0x01, 0xe3, 0x30, 0x82, 0x01, 0x89, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x39,
  0xdd, 0x1f, 0x36, 0xf8, 0x63, 0x2e, 0xa2, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
  0x04, 0x03, 0x02, 0x30, 0x3d, 0x31, 0x25, 0x30, 0x23, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x1c,
  0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x44, 0x65, 0x76, 0x20, 0x50, 0x41, 0x49, 0x20, 0x30,
  0x78, 0x46, 0x46, 0x46, 0x31, 0x20, 0x6e, 0x6f, 0x20, 0x50, 0x49, 0x44, 0x31, 0x14, 0x30, 0x12,
  0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46,
  0x46, 0x31, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x30, 0x30,
  0x30, 0x30, 0x5a, 0x17, 0x0d, 0x34, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x30, 0x30, 0x30,
  0x30, 0x5a, 0x30, 0x50, 0x31, 0x22, 0x30, 0x20, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x19, 0x4d,
  0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x44, 0x65, 0x76, 0x65, 0x6c, 0x6f, 0x70, 0x6d, 0x65, 0x6e,
  0x74, 0x20, 0x44, 0x41, 0x43, 0x20, 0x30, 0x31, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06,
  0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46, 0x46, 0x31, 0x31, 0x14,
  0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x02, 0x0c, 0x04,
  0x38, 0x30, 0x30, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
  0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x16,
  0x77, 0x63, 0x86, 0x1f, 0xe4, 0xe7, 0xfb, 0x2c, 0x1f, 0x10, 0x01, 0x30, 0xc1, 0x6f, 0x12, 0xaa,
  0xad, 0xda, 0xd2, 0xb7, 0x84, 0x43, 0xea, 0x73, 0xe5, 0xdc, 0x3b, 0xde, 0xbb, 0x86, 0x73, 0x61,
  0x8e, 0x24, 0x5c, 0x84, 0x78, 0x85, 0xc5, 0xbe, 0x98, 0xd1, 0x77, 0xd3, 0xeb, 0xb9, 0xed, 0x6a,
  0xf1, 0x3a, 0x58, 0x7d, 0x52, 0x31, 0x49, 0xc2, 0xf0, 0xef, 0x0d, 0x72, 0x8d, 0x7c, 0x64, 0xa3,
  0x60, 0x30, 0x5e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x02, 0x30,
  0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x07,
  0x80, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xe3, 0x8f, 0x96, 0xed,
  0x97, 0x2e, 0x3f, 0x57, 0x02, 0x22, 0xc1, 0x2a, 0x90, 0x1d, 0xf1, 0xa3, 0xa7, 0x96, 0xbb, 0x93,
  0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x63, 0x54, 0x0e,
  0x47, 0xf6, 0x4b, 0x1c, 0x38, 0xd1, 0x38, 0x84, 0xa4, 0x62, 0xd1, 0x6c, 0x19, 0x5d, 0x8f, 0xfb,
  0x3c, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00,
  0x30, 0x45, 0x02, 0x20, 0x02, 0xd5, 0x44, 0xc3, 0xf7, 0x23, 0xb1, 0x14, 0x0d, 0xf3, 0xc5, 0xc3,
  0x43, 0x28, 0xaa, 0x6a, 0x97, 0x0e, 0xb2, 0xb3, 0x8a, 0x08, 0x84, 0xc1, 0x2d, 0x31, 0x92, 0x46,
  0x98, 0x39, 0x8f, 0x2c, 0x02, 0x21, 0x00, 0xd6, 0xbc, 0x59, 0x04, 0xa8, 0xf0, 0xa2, 0xa0, 0xbe,
  0xbc, 0x35, 0x33, 0xee, 0x85, 0xf0, 0x04, 0x9f, 0xae, 0x88, 0xcc, 0xaf, 0xa9, 0xee, 0xee, 0x19,
  0x2b, 0x3a, 0x39, 0xfb, 0x43, 0xc6, 0x36

])


def pretty_print_hex(a, l=16, indent=''):
    """
    Format a list/bytes/bytearray object into a formatted ascii hex string
    """
    s = ''
    a = bytearray(a)
    for x in range(0, len(a), l):
        s += indent + ''.join(['%02X ' % y for y in a[x:x+l]]) + '\n'
    return s

    
def info(dac='dac.der',iface='hid', device='ecc'):
    ATCA_SUCCESS = 0x00

    # Get the target default config
    cfg = eval('cfg_at{}a_{}_default()'.format(atca_names_map.get(device), atca_names_map.get(iface)))


    # Basic Raspberry Pi I2C check
    if 'i2c' == iface and check_if_rpi():
        cfg.cfg.atcai2c.bus = 1

    # Initialize the stack
    assert atcab_init(cfg) == ATCA_SUCCESS
    print('')

    with open(dac_der_file, "rb") as cert_file:
        cert_data = cert_file.read()

    # Create a certdef object from the configuration
    cert_def = atcacert_def_t(**ATCACERT_DEF_DEVICE_CONFIG)

    # Attach the template to the cert_def
    cert_def.cert_template_size = len(ATCACERT_DEF_DEVICE_TEMPLATE_VECTOR)
    cert_def.cert_template = POINTER(c_uint8)(create_string_buffer(bytes(ATCACERT_DEF_DEVICE_TEMPLATE_VECTOR),
                                                                   cert_def.cert_template_size))
                                                                   

    # Write the device certificate
    assert CertStatus.ATCACERT_E_SUCCESS == atcacert_write_cert(cert_def, cert_data, len(cert_data))
    
    atcab_release()
    
'''
    # Read the device certificate
    hex_str = '41:9a:93:15:c2:17:3e:0c:8c:87:6d:03:cc:fc:94:48:52:64:7f:7f:ec:5e:50:82:f4:05:99:28:ec:a8:94:c5:94:15:13:09:ac:63:1e:4c:b0:33:92:af:68:4b:0b:af:b7:e6:5b:3b:81:62:c2:f5:2b:f9:31:b8:e7:7a:aa:82'
    ca_pub_key = bytearray.fromhex(hex_str.replace(':', ''))

    qa_cert_len = AtcaReference(0)

    assert CertStatus.ATCACERT_E_SUCCESS == atcacert_max_cert_size(cert_def, qa_cert_len)
    qa_cert = bytearray(qa_cert_len.value)
    assert CertStatus.ATCACERT_E_SUCCESS == atcacert_read_cert(cert_def, ca_pub_key, qa_cert, qa_cert_len) 

    print('Input: ', len(cert_data))
    print(pretty_print_hex(cert_data))
    print('Output:', qa_cert_len, len(qa_cert))
    print(pretty_print_hex(qa_cert))
    
    assert cert_data == bytes(qa_cert)
    # Free the library
    atcab_release()

'''

if __name__ == '__main__':
    
    if len(sys.argv) < 2:
        print("Usage: python dev_provision.py [DAC_DER_FILE]")
        sys.exit(1)

    dac_der_file = sys.argv[1]

    info(dac_der_file)
    print('\nDone')