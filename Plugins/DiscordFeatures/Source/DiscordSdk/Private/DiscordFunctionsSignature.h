// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordTypes.h"

/* Signature of the functions we call from the Dll */

typedef FRawDiscord::EDiscordResult(*FDiscordCreate)(FRawDiscord::DiscordVersion version, FRawDiscord::DiscordCreateParams* params, FRawDiscord::IDiscordCore** result);
