package com.dogetoreum.qt;

import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;

import org.qtproject.qt5.android.bindings.QtActivity;

import java.io.File;

public class DogetoreumQtActivity extends QtActivity
{
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		final File dogetoreumDir = new File(getFilesDir().getAbsolutePath() + "/.dogetoreumcore");
		if (!dogetoreumDir.exists()) {
			dogetoreumDir.mkdir();
		}


		super.onCreate(savedInstanceState);
	}
}