
package com.gaztin.myapp;

import android.app.Activity;
import android.os.Bundle;

class MyActivity extends Activity
{
	@Override
	protected void onCreate( Bundle savedInstanceState )
	{
		super.onCreate( savedInstanceState );

		System.loadLibrary( "MyApp" );
	}
}
