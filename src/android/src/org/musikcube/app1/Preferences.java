package org.musikcube.app1;

import android.content.Intent;
import android.os.Bundle;
import android.preference.PreferenceActivity;

public class Preferences extends PreferenceActivity {
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		addPreferencesFromResource(R.xml.preferences);
	}

	@Override
	protected void onStop() {
//		Log.v("mC2::PREFS","onStop");
		// TODO Auto-generated method stub
		super.onStop();
		// Let the library know to restart connection
		org.musikcube.core.Library.GetInstance().Restart();
	}
	
	@Override
	protected void onPause() {
//		Log.v("mC2::PREFS","onPause");
		super.onPause();
		org.musikcube.core.Library.GetInstance().RemovePointer();
	}

	@Override
	protected void onResume() {
//		Log.v("mC2::PREFS","onResume");
		super.onResume();
		org.musikcube.core.Library.GetInstance().AddPointer();
		startService(new Intent(this, org.musikcube.app1.Service.class));
	}
	
}
