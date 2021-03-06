/* Copyright 2013 David Axmark

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package com.mosync.internal.android.nfc;

import static com.mosync.internal.generated.MAAPI_consts.MA_NFC_TAG_TYPE_ISO_DEP;

import java.io.IOException;

import android.nfc.tech.IsoDep;

public class IsoDepTag extends NFCTagBase<IsoDep> implements ITransceivable<IsoDep> {

	public static IsoDepTag get(ResourcePool pool, GenericTag tag) {
		IsoDep isoDep = IsoDep.get(tag.getTag());
		return isoDep == null ? null : new IsoDepTag(pool, isoDep);
	}

	private IsoDepTag(ResourcePool pool, IsoDep isoDep) {
		super(pool, isoDep, MA_NFC_TAG_TYPE_ISO_DEP);
	}

	@Override
	public byte[] transceive(byte[] buffer) throws IOException {
		return nativeTag.transceive(buffer);
	}

}
