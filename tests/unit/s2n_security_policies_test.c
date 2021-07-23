/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "s2n_test.h"

#include "tls/s2n_security_policies.h"
#include "tls/s2n_kem.h"
#include "pq-crypto/s2n_pq.h"

int main(int argc, char **argv)
{
    BEGIN_TEST();
    EXPECT_SUCCESS(s2n_disable_tls13());

    EXPECT_TRUE(s2n_array_len(ALL_SUPPORTED_KEM_GROUPS) == S2N_SUPPORTED_KEM_GROUPS_COUNT);

    /* Perform basic checks on all Security Policies. */
    for (size_t policy_index = 0; security_policy_selection[policy_index].version != NULL; policy_index++) {
        const struct s2n_security_policy *security_policy = security_policy_selection[policy_index].security_policy;

        /* TLS 1.3 + PQ checks */
        if (security_policy->kem_preferences->tls13_kem_group_count > 0) {
            /* Ensure that no TLS 1.3 KEM group preference lists go over max supported limit */
            EXPECT_TRUE(security_policy->kem_preferences->tls13_kem_group_count <= S2N_SUPPORTED_KEM_GROUPS_COUNT);

            /* Ensure all TLS 1.3 KEM groups in all policies are in the global list of all supported KEM groups */
            for(int i = 0; i < security_policy->kem_preferences->tls13_kem_group_count; i++) {
                const struct s2n_kem_group *kem_group = security_policy->kem_preferences->tls13_kem_groups[i];

                int kem_group_is_supported = 0;
                for (int j = 0; j < S2N_SUPPORTED_KEM_GROUPS_COUNT; j++) {
                    if (kem_group->iana_id == ALL_SUPPORTED_KEM_GROUPS[j]->iana_id) {
                        kem_group_is_supported = 1;
                        break;
                    }
                }
                EXPECT_TRUE(kem_group_is_supported);
            }
        }
    }

    const struct s2n_security_policy *security_policy = NULL;

    /* Test common known good cipher suites for expected configuration */
    {
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("default", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_FALSE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(0, security_policy->kem_preferences->kem_count);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);
        EXPECT_FALSE(s2n_security_policy_supports_tls13(security_policy));

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("default_tls13", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_FALSE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_security_policy_supports_tls13(security_policy));
        EXPECT_EQUAL(0, security_policy->kem_preferences->kems);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);
        EXPECT_NULL(security_policy->kem_preferences->kems);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("test_all", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(5, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r2r1_2020_07);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("KMS-TLS-1-0-2018-10", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_FALSE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(0, security_policy->kem_preferences->kem_count);
        EXPECT_NULL(security_policy->kem_preferences->kems);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("KMS-PQ-TLS-1-0-2019-06", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(2, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r1);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-SIKE-TEST-TLS-1-0-2019-11", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(1, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_sike_r1);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-SIKE-TEST-TLS-1-0-2020-02", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(2, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_sike_r2r1);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("KMS-PQ-TLS-1-0-2020-02", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(4, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r2r1);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("KMS-PQ-TLS-1-0-2020-07", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(5, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r2r1_2020_07);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-0-2020-12", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(5, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r2r1_2020_07);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r2);
#if EVP_APIS_SUPPORTED
        EXPECT_EQUAL(6, security_policy->kem_preferences->tls13_kem_group_count);
#else
        EXPECT_EQUAL(3, security_policy->kem_preferences->tls13_kem_group_count);
#endif

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-1-2021-05-17", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(7, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r3r2r1_2021_05);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r3r2);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-0-2021-05-18", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(7, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r3r2r1_2021_05);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r3r2);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-0-2021-05-19", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(7, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r3r2r1_2021_05);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r3r2);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-0-2021-05-20", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(7, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r3r2r1_2021_05);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r3r2);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-1-2021-05-21", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(7, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r3r2r1_2021_05);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r3r2);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-0-2021-05-22", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(7, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r3r2r1_2021_05);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r3r2);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-0-2021-05-23", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(7, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r3r2r1_2021_05);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r3r2);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("PQ-TLS-1-0-2021-05-24", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(7, security_policy->kem_preferences->kem_count);
        EXPECT_NOT_NULL(security_policy->kem_preferences->kems);
        EXPECT_EQUAL(security_policy->kem_preferences->kems, pq_kems_r3r2r1_2021_05);
        EXPECT_NOT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(security_policy->kem_preferences->tls13_kem_groups, pq_kem_groups_r3r2);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("20141001", &security_policy));
        EXPECT_FALSE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_FALSE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(0, security_policy->kem_preferences->kem_count);
        EXPECT_NULL(security_policy->kem_preferences->kems);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);

        security_policy = NULL;
        EXPECT_SUCCESS(s2n_find_security_policy_from_version("20201021", &security_policy));
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_FALSE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_EQUAL(0, security_policy->kem_preferences->kem_count);
        EXPECT_NULL(security_policy->kem_preferences->kems);
        EXPECT_NULL(security_policy->kem_preferences->tls13_kem_groups);
        EXPECT_EQUAL(0, security_policy->kem_preferences->tls13_kem_group_count);
    }

    {
        char tls12_only_security_policy_strings[][255] = {
            "default",
            "default_fips",
            "ELBSecurityPolicy-TLS-1-0-2015-04",
            "ELBSecurityPolicy-TLS-1-0-2015-05",
            "ELBSecurityPolicy-2016-08",
            "ELBSecurityPolicy-TLS-1-1-2017-01",
            "ELBSecurityPolicy-TLS-1-2-2017-01",
            "ELBSecurityPolicy-TLS-1-2-Ext-2018-06",
            "ELBSecurityPolicy-FS-2018-06",
            "ELBSecurityPolicy-FS-1-2-2019-08",
            "ELBSecurityPolicy-FS-1-1-2019-08",
            "ELBSecurityPolicy-FS-1-2-Res-2019-08",
            "CloudFront-Upstream",
            "CloudFront-Upstream-TLS-1-0",
            "CloudFront-Upstream-TLS-1-1",
            "CloudFront-Upstream-TLS-1-2",
            /* CloudFront legacy viewer facing policies (max TLS 1.2)  */
            "CloudFront-SSL-v-3-Legacy",
            "CloudFront-TLS-1-0-2014-Legacy",
            "CloudFront-TLS-1-0-2016-Legacy",
            "CloudFront-TLS-1-1-2016-Legacy",
            "CloudFront-TLS-1-2-2018-Legacy",
            "CloudFront-TLS-1-2-2019-Legacy",
            "KMS-TLS-1-0-2018-10",
            "KMS-PQ-TLS-1-0-2019-06",
            "KMS-PQ-TLS-1-0-2020-02",
            "KMS-PQ-TLS-1-0-2020-07",
            "PQ-SIKE-TEST-TLS-1-0-2019-11",
            "PQ-SIKE-TEST-TLS-1-0-2020-02",
            "KMS-FIPS-TLS-1-2-2018-10",
            "20140601",
            "20141001",
            "20150202",
            "20150214",
            "20150306",
            "20160411",
            "20160804",
            "20160824",
            "20170210",
            "20170328",
            "20190214",
            "20170405",
            "20170718",
            "20190120",
            "20190121",
            "20190122",
            "20201021",
            "test_all_fips",
            "test_all_ecdsa",
            "test_ecdsa_priority",
            "test_all_tls12",
        };

        for (size_t i = 0; i < s2n_array_len(tls12_only_security_policy_strings); i++) {
            security_policy = NULL;
            EXPECT_SUCCESS(s2n_find_security_policy_from_version(tls12_only_security_policy_strings[i], &security_policy));
            EXPECT_FALSE(s2n_security_policy_supports_tls13(security_policy));
        }

        char tls13_security_policy_strings[][255] = {
            "default_tls13",
            "test_all",
            "test_all_tls13",
            "20190801",
            "20190802",
            /* CloudFront viewer facing */
            "CloudFront-SSL-v-3",
            "CloudFront-TLS-1-0-2014",
            "CloudFront-TLS-1-0-2016",
            "CloudFront-TLS-1-1-2016",
            "CloudFront-TLS-1-2-2018",
            "CloudFront-TLS-1-2-2019",
            "CloudFront-TLS-1-2-2021",
        };
        for (size_t i = 0; i < s2n_array_len(tls13_security_policy_strings); i++) {
            security_policy = NULL;
            EXPECT_SUCCESS(s2n_find_security_policy_from_version(tls13_security_policy_strings[i], &security_policy));
            EXPECT_TRUE(s2n_security_policy_supports_tls13(security_policy));
        }
    }

    /* Test that null fails */
    {
        security_policy = NULL;
        EXPECT_FALSE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_FALSE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_FALSE(s2n_security_policy_supports_tls13(security_policy));
    }

    /* Test a security policy not on the official list */
    {
        struct s2n_cipher_suite *fake_suites[] = {
            &s2n_ecdhe_bike_rsa_with_aes_256_gcm_sha384,
            &s2n_tls13_chacha20_poly1305_sha256,
        };

        const struct s2n_cipher_preferences fake_cipher_preference = {
            .count = s2n_array_len(fake_suites),
            .suites = fake_suites,
        };

        const struct s2n_kem_preferences fake_kem_preference = {
            .kem_count = 1,
            .kems = NULL,
        };

        const struct s2n_security_policy fake_security_policy = {
            .minimum_protocol_version = S2N_TLS10,
            .cipher_preferences = &fake_cipher_preference,
            .kem_preferences = &fake_kem_preference,
        };

        security_policy = &fake_security_policy;
        EXPECT_TRUE(s2n_ecc_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_pq_kem_is_extension_required(security_policy));
        EXPECT_TRUE(s2n_security_policy_supports_tls13(security_policy));
    }
    {
        struct s2n_config *config = s2n_config_new();

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "default"));
        EXPECT_EQUAL(config->security_policy, &security_policy_20170210);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_20170210);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "20170210"));
        EXPECT_EQUAL(config->security_policy, &security_policy_20170210);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_20170210);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "default_tls13"));
        EXPECT_EQUAL(config->security_policy, &security_policy_20201110);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_20190801);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20200207);
        EXPECT_EQUAL(config->security_policy->certificate_signature_preferences, &s2n_certificate_signature_preferences_20201110);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_20200310);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "20190801"));
        EXPECT_EQUAL(config->security_policy, &security_policy_20190801);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_20190801);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20200207);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_20200310);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "null"));
        EXPECT_EQUAL(config->security_policy, &security_policy_null);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_null);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_null);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_null);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "test_all"));
        EXPECT_EQUAL(config->security_policy, &security_policy_test_all);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_test_all);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20201021);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_test_all);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "test_all_tls12"));
        EXPECT_EQUAL(config->security_policy, &security_policy_test_all_tls12);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_test_all_tls12);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20201021);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_20201021);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "KMS-PQ-TLS-1-0-2020-07"));
        EXPECT_EQUAL(config->security_policy, &security_policy_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "KMS-PQ-TLS-1-0-2020-02"));
        EXPECT_EQUAL(config->security_policy, &security_policy_kms_pq_tls_1_0_2020_02);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_kms_pq_tls_1_0_2020_02);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2020_02);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "KMS-PQ-TLS-1-0-2019-06"));
        EXPECT_EQUAL(config->security_policy, &security_policy_kms_pq_tls_1_0_2019_06);
        EXPECT_EQUAL(config->security_policy->cipher_preferences, &cipher_preferences_kms_pq_tls_1_0_2019_06);
        EXPECT_EQUAL(config->security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2019_06);
        EXPECT_EQUAL(config->security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(config->security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_FAILURE(s2n_config_set_cipher_preferences(config, NULL));

        EXPECT_FAILURE_WITH_ERRNO(s2n_config_set_cipher_preferences(config, "notathing"),
                S2N_ERR_INVALID_SECURITY_POLICY);

        s2n_config_free(config);
    }
    {
        struct s2n_config *config = s2n_config_new();

        struct s2n_connection *conn = s2n_connection_new(S2N_CLIENT);
        s2n_connection_set_config(conn, config);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "default"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_20170210);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_20170210);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "20170210"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_20170210);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_20170210);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "default_tls13"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_20201110);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_20190801);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20200207);
        EXPECT_EQUAL(security_policy->certificate_signature_preferences, &s2n_certificate_signature_preferences_20201110);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_20200310);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "20190801"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_20190801);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_20190801);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_null);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20200207);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_20200310);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "test_all"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_test_all);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_test_all);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20201021);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_test_all);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "test_all_tls12"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_test_all_tls12);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_test_all_tls12);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20201021);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_20201021);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "KMS-PQ-TLS-1-0-2020-07"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2020_07);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "KMS-PQ-TLS-1-0-2020-02"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_kms_pq_tls_1_0_2020_02);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_kms_pq_tls_1_0_2020_02);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2020_02);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_SUCCESS(s2n_connection_set_cipher_preferences(conn, "KMS-PQ-TLS-1-0-2019-06"));
        EXPECT_SUCCESS(s2n_connection_get_security_policy(conn, &security_policy));
        EXPECT_EQUAL(security_policy, &security_policy_kms_pq_tls_1_0_2019_06);
        EXPECT_EQUAL(security_policy->cipher_preferences, &cipher_preferences_kms_pq_tls_1_0_2019_06);
        EXPECT_EQUAL(security_policy->kem_preferences, &kem_preferences_kms_pq_tls_1_0_2019_06);
        EXPECT_EQUAL(security_policy->signature_preferences, &s2n_signature_preferences_20140601);
        EXPECT_EQUAL(security_policy->ecc_preferences, &s2n_ecc_preferences_20140601);

        EXPECT_FAILURE_WITH_ERRNO(s2n_connection_set_cipher_preferences(conn, "notathing"),
                S2N_ERR_INVALID_SECURITY_POLICY);

        s2n_config_free(config);
        s2n_connection_free(conn);
    }

    /* All signature preferences are valid */
    {
        for (int i = 0; security_policy_selection[i].version != NULL; i++) {
            security_policy = security_policy_selection[i].security_policy;
            EXPECT_NOT_NULL(security_policy);
            EXPECT_NOT_NULL(security_policy->signature_preferences);

            for (int j = 0; j < security_policy->signature_preferences->count; j++) {
                const struct s2n_signature_scheme *scheme = security_policy->signature_preferences->signature_schemes[j];

                EXPECT_NOT_NULL(scheme);

                uint8_t max_version = scheme->maximum_protocol_version;
                uint8_t min_version = scheme->minimum_protocol_version;

                EXPECT_TRUE(max_version == S2N_UNKNOWN_PROTOCOL_VERSION || min_version <= max_version);

                /* If scheme will be used for tls1.3 */
                if (max_version == S2N_UNKNOWN_PROTOCOL_VERSION || max_version >= S2N_TLS13) {
                    EXPECT_NOT_EQUAL(scheme->hash_alg, S2N_HASH_SHA1);
                    EXPECT_NOT_EQUAL(scheme->sig_alg, S2N_SIGNATURE_RSA);
                    if (scheme->sig_alg == S2N_SIGNATURE_ECDSA) {
                        EXPECT_NOT_NULL(scheme->signature_curve);
                    }
                }

                /* If scheme will be used for pre-tls1.3 */
                if (min_version < S2N_TLS13) {
                    EXPECT_NULL(scheme->signature_curve);
                    EXPECT_NOT_EQUAL(scheme->sig_alg, S2N_SIGNATURE_RSA_PSS_PSS);
                }
            }
        }
    }

    /* Failure case when s2n_ecc_preference lists contains a curve not present in s2n_all_supported_curves_list */
    {
        const struct s2n_ecc_named_curve test_curve = {
            .iana_id = 12345, 
            .libcrypto_nid = 0, 
            .name = "test_curve", 
            .share_size = 0
        };

        const struct s2n_ecc_named_curve *const s2n_ecc_pref_list_test[] = {
            &test_curve,
        };

        const struct s2n_ecc_preferences s2n_ecc_preferences_new_list = {
            .count = s2n_array_len(s2n_ecc_pref_list_test),
            .ecc_curves = s2n_ecc_pref_list_test,
        };

        EXPECT_FAILURE(s2n_check_ecc_preferences_curves_list(&s2n_ecc_preferences_new_list));
    }

    /* Positive and negative cases for s2n_validate_kem_preferences() */
    {
        EXPECT_FAILURE_WITH_ERRNO(s2n_validate_kem_preferences(NULL, 0), S2N_ERR_NULL);
        EXPECT_FAILURE_WITH_ERRNO(s2n_validate_kem_preferences(&kem_preferences_null, 1), S2N_ERR_INVALID_SECURITY_POLICY);
        EXPECT_SUCCESS(s2n_validate_kem_preferences(&kem_preferences_null, 0));

        const struct s2n_kem_group *test_kem_group_list[] = {
                &s2n_secp256r1_sike_p434_r3
        };

        const struct s2n_kem_preferences invalid_kem_prefs[] = {
            {
                .kem_count = 1,
                .kems = NULL,
                .tls13_kem_group_count = 0,
                .tls13_kem_groups = NULL,
            },
            {
                .kem_count = 0,
                .kems = NULL,
                .tls13_kem_group_count = 1,
                .tls13_kem_groups = NULL,
            },
            {
                .kem_count = 0,
                .kems = pq_kems_r1,
                .tls13_kem_group_count = 0,
                .tls13_kem_groups = NULL,
            },
            {
                .kem_count = 0,
                .kems = NULL,
                .tls13_kem_group_count = 0,
                .tls13_kem_groups = test_kem_group_list,
            },
        };

        for (size_t i = 0; i < s2n_array_len(invalid_kem_prefs); i++) {
            EXPECT_FAILURE_WITH_ERRNO(s2n_validate_kem_preferences(&invalid_kem_prefs[i], 1), S2N_ERR_INVALID_SECURITY_POLICY);
        }

        EXPECT_FAILURE_WITH_ERRNO(s2n_validate_kem_preferences(&kem_preferences_kms_pq_tls_1_0_2020_07, 0), S2N_ERR_INVALID_SECURITY_POLICY);
        EXPECT_SUCCESS(s2n_validate_kem_preferences(&kem_preferences_kms_pq_tls_1_0_2020_07, 1));
    }

    /* Checks that NUM_RSA_PSS_SCHEMES accurately represents the number of rsa_pss signature schemes usable in a
     * certificate_signature_preferences list */
    {
        for (size_t i = 0; security_policy_selection[i].version != NULL; i++) {
            security_policy = security_policy_selection[i].security_policy;
            EXPECT_NOT_NULL(security_policy);

            if (security_policy->certificate_signature_preferences != NULL) {
                size_t num_rsa_pss = 0;
                for (size_t j = 0; j < security_policy->certificate_signature_preferences->count; j++) {
                    if (security_policy->certificate_signature_preferences->signature_schemes[j]->libcrypto_nid == NID_rsassaPss) {
                        num_rsa_pss +=1;
                    }
                }
                EXPECT_TRUE(num_rsa_pss <= NUM_RSA_PSS_SCHEMES);
            }
        }
    }

    /* s2n_validate_certificate_signature_preferences will succeed if there are no rsa_pss schemes in the preference list */
    {
        const struct s2n_signature_scheme* const test_sig_scheme_pref_list[] = {
            &s2n_rsa_pkcs1_sha256,
        };

        const struct s2n_signature_preferences test_certificate_signature_preferences = {
            .count = s2n_array_len(test_sig_scheme_pref_list),
            .signature_schemes = test_sig_scheme_pref_list,
        };

        EXPECT_OK(s2n_validate_certificate_signature_preferences(&test_certificate_signature_preferences));
    }

    /* s2n_validate_certificate_signature_preferences will succeed if all rsa_pss schemes are included in the preference list */
    {
        const struct s2n_signature_scheme* const test_sig_scheme_pref_list[] = {
            &s2n_rsa_pss_pss_sha256,
            &s2n_rsa_pss_pss_sha384,
            &s2n_rsa_pss_pss_sha512,
            &s2n_rsa_pss_rsae_sha256,
            &s2n_rsa_pss_rsae_sha384,
            &s2n_rsa_pss_rsae_sha512,
        };

        const struct s2n_signature_preferences test_certificate_signature_preferences = {
            .count = s2n_array_len(test_sig_scheme_pref_list),
            .signature_schemes = test_sig_scheme_pref_list,
        };

        EXPECT_OK(s2n_validate_certificate_signature_preferences(&test_certificate_signature_preferences));
    }

    /* s2n_validate_certificate_signature_preferences will fail if not all rsa_pss schemes are included in the preference list */
    {
        const struct s2n_signature_scheme* const test_sig_scheme_pref_list[] = {
            &s2n_rsa_pss_pss_sha256,
            &s2n_rsa_pss_pss_sha384,
        };

        const struct s2n_signature_preferences test_certificate_signature_preferences = {
            .count = s2n_array_len(test_sig_scheme_pref_list),
            .signature_schemes = test_sig_scheme_pref_list,
        };

        EXPECT_ERROR_WITH_ERRNO(s2n_validate_certificate_signature_preferences(&test_certificate_signature_preferences), S2N_ERR_INVALID_SECURITY_POLICY);
    }

    END_TEST();
}
