/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package app.usp;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.Toast;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;

public class Main extends Activity
{
	private TableLayout layout;
	private TableRow row1, row2;
	private app.usp.View view;
	private Control control;
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		Emulator.the.InitRom(0, BinRes(R.raw.sos128));
		Emulator.the.InitRom(1, BinRes(R.raw.sos48));
		Emulator.the.InitRom(2, BinRes(R.raw.service));
		Emulator.the.InitRom(3, BinRes(R.raw.dos513f));
		Emulator.the.InitFont(BinRes(R.raw.spxtrm4f));
		Emulator.the.Init(getFilesDir().getAbsolutePath());
		Context c = getApplicationContext();
		view = new app.usp.View(c);
		control = new Control(c);
		layout = new TableLayout(c);
		row1 = new TableRow(c);
		row2 = new TableRow(c);
		row1.setGravity(Gravity.CENTER);
		row2.setGravity(Gravity.CENTER);
		layout.setGravity(Gravity.BOTTOM);
		setContentView(layout);
		UpdateOrientation(getResources().getConfiguration());
		String file = Uri.parse(getIntent().toUri(0)).getPath();
		if(file.length() != 0)
		{
			Toast.makeText(getApplicationContext(), "Opening \"" + file + "\"", Toast.LENGTH_LONG).show();
			Emulator.the.Open(file);
		}
    }
    @Override
	public void onDestroy()
	{
		Emulator.the.Done();
		super.onDestroy();
    	android.os.Process.killProcess(android.os.Process.myPid());
	}
	private void UpdateOrientation(Configuration config)
	{
		row1.removeAllViews();
		row2.removeAllViews();
		layout.removeAllViews();

		DisplayMetrics dm = getResources().getDisplayMetrics();
		int w = dm.widthPixels;
		int h = dm.heightPixels;
		final int zoom = Emulator.the.GetOptionInt(Preferences.select_zoom_id);
		view.SetFiltering(Emulator.the.GetOptionBool(Preferences.filtering_id));
		view.SetSkipFrames(Emulator.the.GetOptionInt(Preferences.select_skip_frames_id));
		if(config.orientation == Configuration.ORIENTATION_LANDSCAPE)
		{
			view.SetZoom(zoom, w, h);
			row1.addView(control, new TableRow.LayoutParams());
			row1.addView(view, new TableRow.LayoutParams());
			layout.addView(row1);
		}
		else
		{
			view.SetZoom(zoom, w, h);
			row1.addView(view, new TableRow.LayoutParams());
			row2.addView(control, new TableRow.LayoutParams());
			layout.addView(row1);
			layout.addView(row2);
		}
		control.requestFocus();
		view.setKeepScreenOn(true);
	}
    @Override
	protected void onResume()
	{
		super.onResume();
		view.OnResume();
		control.OnResume();
	}
    @Override
	protected void onPause()
	{
		Emulator.the.StoreOptions();
		view.OnPause();
		control.OnPause();
		super.onPause();
	}
    @Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		super.onConfigurationChanged(newConfig);
		UpdateOrientation(newConfig);
	}
    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
    	getMenuInflater().inflate(R.menu.menu, menu);		
    	return true;
    }
    static final int A_FILE_SELECTOR = 0;
    static final int A_PREFERENCES = 1;
    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
    	switch(item.getItemId())
    	{
    	case R.id.open_file:	startActivityForResult(new Intent(this, FileOpen.class), A_FILE_SELECTOR); return true;
		case R.id.save_state:	Emulator.the.SaveState(); 		return true;
		case R.id.load_state:	Emulator.the.LoadState(); 		return true;
		case R.id.reset:		Emulator.the.Reset(); 			return true;
    	case R.id.preferences:	startActivityForResult(new Intent(this, Preferences.class), A_PREFERENCES); return true;
		case R.id.quit:			Exit(); 						return true;
    	}
    	return super.onOptionsItemSelected(item);
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        if(requestCode == A_PREFERENCES)
        {
        	UpdateOrientation(getResources().getConfiguration());
        }
    }
    final private void Exit()
    {
    	finish();
    }
	final private ByteBuffer BinRes(int id)
	{
		InputStream is = getResources().openRawResource(id);
		byte[] data = null;
		try
		{
			data = new byte[is.available()];
			is.read(data);
		}
		catch(IOException e)
		{}
		ByteBuffer bb = ByteBuffer.allocateDirect(data.length);
		bb.put(data);
		bb.rewind();
		return bb;
	}
}
