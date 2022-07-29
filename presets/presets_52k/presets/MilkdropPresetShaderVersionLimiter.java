import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.ArrayList;

public class MilkdropPresetShaderVersionLimiter {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		File path = new File(".");
		System.out.println("path: " + path.getAbsolutePath());

		String[] entries = path.list();
		if (null == entries) {
			System.out.println("Empty Directory");
			return;
		}
		System.out.println("number of files: " + entries.length);

		FilenameFilter milkdropPresetFilenameFilter = new FilenameFilter() {
			public boolean accept(File dir, String name) {
				return name.endsWith(".milk");
			}
		};
		File[] presetFiles = path.listFiles(milkdropPresetFilenameFilter);
		if (presetFiles != null) {
			System.out.println("number of presets: " + presetFiles.length);

			for (int i = 0; i < presetFiles.length; i++) {
				File presetFile = presetFiles[i];
				if (hasWrongShaderVersion(presetFile)) {
					correctShaderVersion(presetFile);
				}
			}
		} else {
			System.out.println("No Milkdrop Presets In This Directory!");
			return;
		}
	}

	private static void correctShaderVersion(File presetFile) {
		ArrayList<String> lines = new ArrayList<String>();
		try {
			BufferedReader reader = new BufferedReader(new FileReader(
					presetFile));
			for (String line = reader.readLine(); line != null; line = reader
					.readLine()) {
				if (line.equals("PSVERSION=4")) {
					line = "PSVERSION=3";
				}
				if (line.equals("PSVERSION_WARP=4")) {
					line = "PSVERSION_WARP=3";
				}
				if (line.equals("PSVERSION_COMP=4")) {
					line = "PSVERSION_COMP=3";
				}
				lines.add(line);
			}
			reader.close();

			BufferedWriter writer = new BufferedWriter(new FileWriter(
					presetFile));
			writer.flush();
			for (int i = 0; i < lines.size(); i++) {
				writer.write(lines.get(i));
				if (i != lines.size() - 1) {
					writer.newLine();
				}
			}
			writer.close();
			System.out.println("file corrected: " + presetFile.getName());
		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	private static boolean hasWrongShaderVersion(File presetFile) {
		if (!presetFile.getName().endsWith(".milk"))
			return false;
		try {
			BufferedReader reader = new BufferedReader(new FileReader(
					presetFile));
			for (int i = 0; i < 4; i++) {
				String line = reader.readLine();
				if (line == null){
					System.out.println("oh oh, empty");
					return false;
				}
				if (line.equals("PSVERSION=4"))
					return true;
				if (line.equals("PSVERSION_WARP=4"))
					return true;
				if (line.equals("PSVERSION_COMP=4"))
					return true;
			}

			reader.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return false;
	}

}
